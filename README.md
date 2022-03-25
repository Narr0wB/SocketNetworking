# *Project name*
*Project name*  is a C++ - python TCP socket application for video streaming and executing commands in the server side of the project

## TODO List
1 - Fix debug printouts, make a standard for debug prints and maybe change/add color? *something like "[DEBUG] something!" for all functions that have a debug option*

2 - Assign better names to varaibles, make a standard for variable names in the C++ side of the code and a another standart for the python side

3 - Add comments that explain what the code is doing, like at the start of each function

4 - Modify the packet header system from *0x0 0x0 0x0 0x0 | 0x0 | 0x0 |* to *0x0 0x0 0x0 0x0 | 0x0 0x0 | 0x0* packetPayloadSize | nOfPacketsLeft | typeOfRequest

5 - In the server code, get rid of the call to ```os.system()``` to execute client commands and replace it with a child Popen process in order to allow file exploring

6 - Add functionality to send and receive files

7 - Make the change from TCP stream socket to UDP stream *how did i miss this?*

8 - For the client side of the code, implement a gui (ImGUI library or others)
