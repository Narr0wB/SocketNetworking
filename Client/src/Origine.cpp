#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>

#include "socketfuncs.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
	
	SOCKET sckt = Message::createSocket("127.0.0.1", "8081");

	char Command[2000]; 
	while (1) {
		fgets(Command, 2000, stdin); Command[strlen(Command)-1] = '\0';
		Message::sendPackets(sckt, Command, false);
		std::vector<char> response = Message::recvPackets(sckt, true, true);
		printf("%s, %d", response.data(), response.size());
	}


	return 0;
}