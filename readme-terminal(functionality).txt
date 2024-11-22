IF directory named cgi-bin has a program, compile it first, then it should show output on browser.

I have implemented Errors 400, 403, 404, 500, and 501

If directory other than cgi-bin exist then it should list(ls) contents of directory.

Below are functionality from terminal. Not shown but curl -X POST will give "Not Implemented" error.

Script started on 2024-11-21 20:40:52-08:00 [TERM="xterm-256color" TTY="/dev/pts/1" COLUMNS="94" LINES="51"]
[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ sc[K[Kcurl -i http://localhost:6000/[K[K[K[K[K7000/index
[?2004l
HTTP/1.0 404 Not Found

[1mContent-Type[0m: text/html

[1mContent-Length[0m: 15



File not Found
[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ curl -i http://localhost:7000/index.html
[?2004l
HTTP/1.0 200 OK

[1mContent-Type[0m: text/html

[1mContent-Length[0m: 19



357 - assignment-5
[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ curl -I http://localhost:8000/index.html
[?2004l
HTTP/1.0 200 OK

[1mContent-Type[0m: text/html

[1mContent-Length[0m: 19



[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ curl -i http://localhost:8000/index.html
[?2004l
HTTP/1.0 200 OK

[1mContent-Type[0m: text/html

[1mContent-Length[0m: 19



357 - assignment-5
[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ nc local ho[K[K[Khost 8000
[?2004l

HTTP/1.0 400 Bad Request

Content-Type: text/html

Content-Length: 21



Wrong Format Request

[?2004h]0;senith@senith-Swift-SF314-54G: ~/Desktop/syst_assi5[01;32msenith@senith-Swift-SF314-54G[00m:[01;34m~/Desktop/syst_assi5[00m$ exit
[?2004l
exit

Script done on 2024-11-21 20:45:29-08:00 [COMMAND_EXIT_CODE="0"]
