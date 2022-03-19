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
#include "videomodules.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
	
	SOCKET sckt = Message::createSocket("127.0.0.1", "8081");

	std::string command;
	bool readCommand = false;
	while (1) {
		std::getline(cin, command);
		readCommand = true;
		
		if (command.find("video")) {
			if 
		}
		else {
			std::vector<unsigned char> Input(command.begin(), command.end());
			Message::sendPackets(sckt, Input, "c", true);
			std::vector<unsigned char> response = Message::recvPackets(sckt, true);
			printf("%s\n", response.data());
		}
		
	}


	return 0;
}