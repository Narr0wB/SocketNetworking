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

	void showFrames(SOCKET sckt, bool askForNextFrame)
	{
		std::vector<unsigned char> frameBuffer;

		frameBuffer = receiveFrameBuffers(sckt, askForNextFrame);
		std::cout << frameBuffer.size();
		std::ofstream binFile("received.png", std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)frameBuffer.data(), frameBuffer.size());
			binFile.close();
		}

}
}
