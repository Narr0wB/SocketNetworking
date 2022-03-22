#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <thread>

#include "socketfuncs.h"
#include "videomodules.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
	
	SOCKET sckt = Message::createSocket("127.0.0.1", "8081");

	std::string command;
	std::atomic<bool> done(true);
	std::thread videoShow;

	while (1) {
		std::getline(cin, command);

		if (command.find("video") != std::string::npos) {
			if (command.find("start") != std::string::npos && done) {
				std::vector<unsigned char> Input(command.begin(), command.end());
				Message::sendPackets(sckt, Input, "v", true);
				command = "";

				videoShow = std::thread(Video::showFrames, command, sckt, std::ref(done));
			}
		}
		else if (done) {
			std::vector<unsigned char> Input(command.begin(), command.end());
			Message::sendPackets(sckt, Input, "c", true);
			std::vector<unsigned char> response = Message::recvPackets(sckt, true);
			printf("%s\n", response.data());
		}
		command = "";
	}


	return 0;
}