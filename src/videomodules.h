
#include "socketfuncs.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <stdlib.h>
#include <fstream>

namespace Video {

	std::vector<unsigned char> receiveFrameBuffers(SOCKET sckt, bool askForNextFrame);

	void showFrames(std::string command, SOCKET sckt, std::atomic<bool>& isDone);

}

