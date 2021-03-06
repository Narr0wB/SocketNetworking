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
#define PACKET_SIZE 50000

namespace Message 
{
	SOCKET createSocket(LPCSTR IPAdress, LPCSTR Port, bool makingClient, bool debug);

	std::vector<unsigned char> recvAll(const SOCKET& sckt, unsigned int nOfBytesToRecv);

	std::vector<unsigned char> recvMsg(const SOCKET& sckt, bool debug);

	void sendMsg(const SOCKET& sckt, std::vector<unsigned char> dataToSend, const char* typeOfRequest, bool debug);
}


