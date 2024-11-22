#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

#define log 10
#define cgi_dir "cgi-bin"
#define Path_max 4096

void client(int client_fd);
void cgi_request(int client_fd, char *path, char *args);
void dir_request(int client_fd, char *path);
int check_path(const char *path);

void error_message(int client_fd, const char *status, const char *message) {
    char respose[1024];
    size_t body_len = strlen(message) +1;
    snprintf(respose, sizeof(respose),
        "HTTP/1.0 %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "\r\n"
        "%s\n", status, body_len, message);
    write(client_fd, respose, strlen(respose));
}



int main(int argc, char *argv[]) {

    if(argc !=2) {
        fprintf(stderr, "error in args\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port < 1024 || port > 65535) {
        fprintf(stderr, "Error: port num between 1024 and 65535\n");
        exit(EXIT_FAILURE);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd ==-1) {
        perror("error socket\n");
        exit(EXIT_FAILURE);
    }

    struct  sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("error bind\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, log) == -1) {
        perror("error listen\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("HTTP server listening on port %d\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr,&client_len);

        if (client_fd == -1) {
            perror("error accept\n");
            continue;
        }

        printf("Connection started with %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t pid = fork();
        if(pid == 0) {
            close(server_fd);
            client(client_fd);
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0) {
            close(client_fd);
        }
        else {
            perror("error fork");
        }

        while(waitpid(-1, NULL, WNOHANG) > 0);
    }

    close(server_fd);
    return 0;
    
}

void client(int client_fd) {
    char buffer[1024] = {0};
    ssize_t byte_r = read(client_fd, buffer, sizeof(buffer)-1);

    if (byte_r <0) {
        close(client_fd);
        return;
    }

    printf("Request recieved: \n %s \n", buffer);

    char method[8], path[1024], protocol[16];
    if(sscanf(buffer, "%7s %1023s %15s", method, path, protocol) != 3){
        error_message(client_fd, "400 Bad Request", "Wrong Format Request");
        return;
    }

    if (path[0] =='/') {
        memmove(path, path +1, strlen(path));
    }

    if(strstr(path, "..")!= NULL) {
        error_message(client_fd, "403 Permisson Denied", "Access to parent directoy denied");
        return;
    }

    if(path[0] == '\0') {
        error_message(client_fd, "400 Bad Request", "Filename must be given");
        return;
    }

    if (strncmp(path, cgi_dir "/", strlen(cgi_dir) +1) == 0) {

        if(check_path(path) == 0) {
            error_message(client_fd, "403 Permision Denied", "Invalid path");
            return;
        }

        char *args = strchr(path,'?');
        if(args) {
            *args = '\0';
            args++;
        }
        cgi_request(client_fd, path+ strlen(cgi_dir) +1, args);
        return;
    }

    if (strchr(path, '/') != NULL) {
        dir_request(client_fd, path);
        return;
    }

    if(strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        error_message(client_fd, "501 Not Implemented", "Only GET and HEAD methods supported");
        return;
    }

    int file_fd = open(path, O_RDONLY);
    if (file_fd == -1) {
        error_message(client_fd, "404 Not Found", "File not Found");
        return;
    }

    struct stat st;
    
    if(fstat(file_fd,&st)==1 || !S_ISREG(st.st_mode)) {
        error_message(client_fd, "403 Permission Denied", "Access denied(wrong file type)");
        close(file_fd);
        return;
    }

    size_t file_size =st.st_size;

    char res_header[1024];
    snprintf(res_header, sizeof(res_header), 
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", file_size);
    

    if(strcmp(method, "GET") == 0) {
        
        if(write(client_fd, res_header, strlen(res_header)) == -1) {
            close(file_fd);
            return;
        }

        char file_buffer[1024];

        ssize_t bytes_read;
        while ((bytes_read = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
            if(write(client_fd, file_buffer, bytes_read) == -1) {
                close(file_fd);
                return;
            }
        }
    }else if (strcmp(method, "HEAD") == 0) {
        write(client_fd, res_header, strlen(res_header));

    }
    close(file_fd);
}

void dir_request(int client_fd, char *path) {

    DIR *dir = opendir(path);
    if(!dir) {
        error_message(client_fd, "404 Not Found", "Directory not found");
        return;
    }

    struct dirent *entry;
    char res[4096] = "Directory: \n";
    while ((entry = readdir(dir)) != NULL)
    {
        strcat(res,entry->d_name);
        strcat(res, "\n");
    }
    closedir(dir);

    char res_header[1024];
    snprintf(res_header, sizeof(res_header),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", strlen(res));

    write(client_fd, res_header, strlen(res_header));
    write(client_fd, res, strlen(res));
    
    
}

void cgi_request(int client_fd, char *path, char *args) {
    char pr_path[1024];
    snprintf(pr_path, sizeof(pr_path), "%s/%s", cgi_dir, path);

    if(access(pr_path, X_OK) != 0) {
        error_message(client_fd, "404 Not Found", "Program not found");
        return;
    }

    char temp_f[64];
    snprintf(temp_f, sizeof(temp_f), "/tmp/cgi_output_%d", getpid());

    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(temp_f, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("Failed temp file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);

        char *ar_list[128] = {pr_path};
        int x = 1;
        if (args) {
            char *arg = strtok(args, "&");
            while(arg && x < 127) {
                ar_list[x++] = arg;
                arg = strtok(NULL, "&");
            }
        }

        ar_list[x] = NULL;

        execvp(pr_path, ar_list);
        perror("exec faied");

        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        int stats;
        waitpid(pid, &stats, 0);

        int fd = open(temp_f, O_RDONLY);
        if (fd == -1) {
            error_message(client_fd, "500 Internal Error", "Failed to read output");
            return;
        }

        struct stat st;
        fstat(fd, &st);
        size_t f_size = st.st_size;

        char res_header[1024];
        snprintf(res_header, sizeof(res_header),
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %ld\r\n"
            "\r\n", f_size);
        
        write(client_fd, res_header, strlen(res_header));

        char buffer[1024];
        ssize_t b_read;
        while((b_read = read(fd, buffer, sizeof(buffer))) > 0) {
            write(client_fd, buffer, b_read);
        }

        close(fd);

        unlink(temp_f);
        
    }
    else {
        error_message(client_fd, "500 Internal Error", "Fork failed");
    }
}

int check_path(const char *path) {
    char r_path[Path_max];

    if (strstr(path, "..") != NULL) {
        return 0;
    }

    if (realpath(path, r_path) == NULL) {
        return 0;
    }

    char cwd[Path_max];
    if (getcwd(cwd, sizeof(cwd)) ==NULL) {
        perror("error getcwd");
        return 0;
    }

    if (strncmp(cwd, r_path, strlen(cwd)) != 0) {
        return 0;
    }
    return 1; //valid path
}
