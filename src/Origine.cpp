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
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	SOCKET sckt = Message::createSocket("127.0.0.1", "8081", true, true);

	std::string command;
	std::shared_ptr<safeString> actualCommand = std::make_shared<safeString>();
	std::thread getCommandThread = std::thread(getCommand, actualCommand);
	std::string s_continueCommand = "continue";
	std::vector<unsigned char> c_continueCommand(s_continueCommand.begin(), s_continueCommand.end());
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
				std::cout << currentDir.data();
			}

			isReceivingVideo = true;
			Video::showFrames(sckt, true, false);
		}
		// If the command is getfile
		if (command.find("getfile") != std::string::npos) {
			// If the client is already receiving video
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, false);
				
				File::getFile(sckt, command, true);
				std::cout << currentDir.data();

				command = "video start";
				(*actualCommand).setData(command);

				Message::sendMsg(sckt, c_continueCommand, "v", false);
			}
			else 
			{
				File::getFile(sckt, command, true);
				std::cout << currentDir.data();

				command = "";
				(*actualCommand).setData(command);
			}
		}
		if (command.find("sendfile") != std::string::npos) 
		{
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, false);

				File::sendFile(sckt, command, true);
				std::cout << currentDir.data();

				command = "video start";
				(*actualCommand).setData(command);

				Message::sendMsg(sckt, c_continueCommand, "v", false);
			}
			else 
			{
				File::sendFile(sckt, command, true);
				std::cout << currentDir.data();

				command = "";
				(*actualCommand).setData(command);
			}
		}
		if (command.find("video") == std::string::npos && command.find("start") == std::string::npos && command != "") 
		{
			if (isReceivingVideo) 
			{
				Video::showFrames(sckt, false, false);
			
				std::vector<unsigned char> inputCommand(command.begin(), command.end());
				Message::sendMsg(sckt, inputCommand, "c", true);

				auto response = Message::recvMsg(sckt, true);
				std::cout << response.data();

				command = "video start";
				(*actualCommand).setData(command);

				Message::sendMsg(sckt, c_continueCommand, "v", false);
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
				Video::showFrames(sckt, false, false);

				std::vector<unsigned char> inputCommand(command.begin(), command.end());
				Message::sendMsg(sckt, inputCommand, "v", true);

				auto response = Message::recvMsg(sckt, true);
				std::cout << response.data() << std::endl;

				isReceivingVideo = false;
				cv::destroyAllWindows();
			}
			else 
			{
				std::cout << "[ERROR] The video stream has not started yet!" << std::endl;
			}
			std::cout << currentDir.data();
			command = "";
			(*actualCommand).setData(command);
		}
		if (command.find("debug") != std::string::npos) {

		}
	}


	return 0;
}