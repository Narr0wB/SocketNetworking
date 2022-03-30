#include "videomodules.h"


namespace Video {
	// Receives frame buffers and if asked to continue, it will send the command to the server (the "continue" command)
	std::vector<unsigned char> receiveFrameBuffers(const SOCKET& sckt, bool askForNextFrame, bool debug)
	{
		std::string continueCommand = "continue";
		std::vector<unsigned char> frameBuffer;

		frameBuffer = Message::recvMsg(sckt, debug);
		if (askForNextFrame) {
			std::vector<unsigned char> Input(continueCommand.begin(), continueCommand.end());
			Message::sendMsg(sckt, Input, "v", debug);
		}

		return frameBuffer;
	}

	// Still have to implement a way to display the image consistently --> OpenCV, OpenGL, GLFW, ImGui...
	void showFrames(const SOCKET& sckt, bool askForNextFrame, bool debug)
	{
		std::vector<unsigned char> frameBuffer;

		frameBuffer = receiveFrameBuffers(sckt, askForNextFrame, debug);
		// Temporary solution
		std::cout << frameBuffer.size();
		std::ofstream binFile("received.png", std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)frameBuffer.data(), frameBuffer.size());
			binFile.close();
		}

}
}
