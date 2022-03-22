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

	void showFrames(std::string command, SOCKET sckt, std::atomic<bool>& isDone)
	{
		isDone = false;
		std::vector<unsigned char> frameBuffer;
		if (1) {
			if (command == "") {
				frameBuffer = receiveFrameBuffers(sckt, false);
				std::cout << frameBuffer.size();
				std::fstream imgout;
				imgout.open("s.png", std::ios::out);
				imgout.write((char*)frameBuffer.data(), frameBuffer.size());
				imgout.close(); 
			}
			else if (command.find("stop") != std::string::npos) {
				isDone = true;

				std::vector<unsigned char> response = Message::recvPackets(sckt, true);
				printf("%s\n", response.data());

				return;
			}
			else if (command.find("start") == std::string::npos && command.find("video") == std::string::npos) {
				frameBuffer = receiveFrameBuffers(sckt, false);
				cv::Mat frame = cv::imdecode(cv::Mat(1080, 1920, CV_8UC3, frameBuffer.data()), -1);
				cv::imshow("Video", frame);
				cv::waitKey(0);

				std::vector<unsigned char> Input(command.begin(), command.end());
				Message::sendPackets(sckt, Input, "c", true);

				std::vector<unsigned char> response = Message::recvPackets(sckt, true);
				printf("%s\n", response.data());
			}
		}
	}
}
