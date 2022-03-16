#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define MAX_DATA_RECEIVABLE 1000

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")
#define PACKET_SIZE 1024

namespace Message {
	SOCKET createSocket(LPCSTR IPAdress, LPCSTR Port);

	std::vector<char> recvAll(SOCKET sckt, int n);

	std::vector<char> recvPackets(SOCKET sckt, bool debug, bool receivingString);

	void sendPackets(SOCKET sckt, char* command, bool debug);
}


