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
#include "filemodules.h"

#pragma comment(lib, "Ws2_32.lib")

void getCommand(std::shared_ptr<safeString> sharedCommand) 
{
	std::string command;
	while (1) 
	{
		std::getline(std::cin, command);
		(*sharedCommand).setData(command);
	}
}

int main() 
{
	
	SOCKET sckt = Message::createSocket("127.0.0.1", "8081", true, true);

	std::string command;
	std::shared_ptr<safeString> actualCommand = std::make_shared<safeString>();
	std::thread getCommandThread = std::thread(getCommand, actualCommand);
	bool isReceivingVideo = false;

	std::vector<unsigned char> currentDir = Message::recvMsg(sckt, false);
	std::cout << currentDir.data();

	while (true) 
	{
		command = (*actualCommand).getData();
		if (command.find("video") != std::string::npos && command.find("start") != std::string::npos) 
		{
			if (!isReceivingVideo) 
			{
				std::vector<unsigned char> startVideoCommand(command.begin(), command.end());
				Message::sendMsg(sckt, startVideoCommand, "v", false);
			}

			isReceivingVideo = true;
			Video::showFrames(sckt, true, true);
		}
		if (command.find("getfile") != std::string::npos) {
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, true);
				
				File::getFile(sckt, command, true);

				command = "video start";
				(*actualCommand).setData(command);
			}
			else 
			{
				File::getFile(sckt, command, true);

				command = "";
				(*actualCommand).setData(command);
			}
		}
		if (command.find("sendfile") != std::string::npos) 
		{
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, true);

				File::sendFile(sckt, command, true);

				command = "video start";
				(*actualCommand).setData(command);
			}
			else 
			{
				File::sendFile(sckt, command, true);

				command = "";
				(*actualCommand).setData(command);
			}
		}
		if (command.find("video") == std::string::npos && command.find("start") == std::string::npos && command != "") 
		{
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, true);
			
				std::vector<unsigned char> inputCommand(command.begin(), command.end());
				Message::sendMsg(sckt, inputCommand, "c", true);

				auto response = Message::recvMsg(sckt, true);
				std::cout << response.data();

				command = "video start";
				(*actualCommand).setData(command);
			}
			else 
			{
				std::vector<unsigned char> inputCommand(command.begin(), command.end());
				Message::sendMsg(sckt, inputCommand, "c", true);

				auto response = Message::recvMsg(sckt, true);
				std::cout << response.data();

				command = "";
				(*actualCommand).setData(command);
			}

		}
		if (command.find("video") != std::string::npos && command.find("stop") != std::string::npos) 
		{
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, true);

				std::vector<unsigned char> inputCommand(command.begin(), command.end());
				Message::sendMsg(sckt, inputCommand, "v", true);

				auto response = Message::recvMsg(sckt, true);
				std::cout << response.data() << std::endl;

				isReceivingVideo = false;

			}
			else 
			{
				std::cout << "[ERROR] The video stream has not started yet!" << std::endl;
			}
			command = "";
			(*actualCommand).setData(command);
		}
	}


	return 0;
}