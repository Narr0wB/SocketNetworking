#include "videomodules.h"


namespace Video {
	std::vector<unsigned char> receiveFrameBuffers(SOCKET sckt, bool askForNextFrame)
	{
		std::string continueCommand = "continue";
		std::vector<unsigned char> frame;

		frame = Message::recvPackets(sckt, true);
		if (askForNextFrame) {
			std::vector<unsigned char> Input(continueCommand.begin(), continueCommand.end());
			Message::sendPackets(sckt, Input, "v", true);
		}

		return frame;
	}

	void showFrames(std::shared_ptr<safeString> actualCommand, SOCKET sckt, std::atomic<bool>& isDone)
	{
		isDone = false;
		std::vector<unsigned char> frameBuffer;
		std::string command;
		bool doneIt = false;
		while (1) {
			command = (*actualCommand).getData();
			std::cout << "c" << command;
			if (command == "") {
				frameBuffer = receiveFrameBuffers(sckt, true);
				std::cout << frameBuffer.size();
				std::ofstream binFile("received.png", std::ios::out | std::ios::binary);
				if (binFile.is_open())
				{
					binFile.write((char*)frameBuffer.data(), frameBuffer.size());
					binFile.close();
				}

			}
			else if (command.find("stop") != std::string::npos) {
				isDone = true;

				std::vector<unsigned char> response = Message::recvPackets(sckt, true);
				printf("%s\n", response.data());

				return;
			}
			else if (command.find("start") == std::string::npos && command.find("video") == std::string::npos && !doneIt) {
				frameBuffer = receiveFrameBuffers(sckt, false);

				std::vector<unsigned char> Input(command.begin(), command.end());
				Message::sendPackets(sckt, Input, "c", true);

				std::vector<unsigned char> response = Message::recvPackets(sckt, true);
				printf("%s\n", response.data());
				doneIt = true;
			}
		}
	}
}
