# TCP Remote access
This project is about a TCP client (**written in C++**) - server (**written in Python**) application that allows for a remote shell, file transfer, and screen sharing.

# List of application-specific commands
Screensharing:

- **start** - starts the video feed
- **stop** - stops the video feed
   

File transfer

 - **getfile [filename | filepath] [savepath]** - retrieves a file present in the server and saves it at a specific path
&emsp;&emsp;**[filename | filepath]** - the path of the file on the server
&emsp;&emsp;**[savepath]** - where on the client to save the file

- **sendfile [filepath] [OPTIONAL: filedestinationpath]** - sends a file present in the client and saves it at a specific path
&emsp;&emsp;**[filepath]** - the path of the file to be sent (on the client)
&emsp;&emsp;**[OPTIONAL: filedestinationpath]** - where on the server to save the file (if the path is not put, it will be saved in 
&emsp;&emsp;the current working directory of the reverse shell)
