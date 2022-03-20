
#include "socketfuncs.h"
#include <stdlib.h>

namespace Video {

	std::vector<unsigned char> receiveFrameBuffers(SOCKET sckt, bool askForNextFrame);

	void showFrames(std::string command, SOCKET sckt, std::atomic<bool>& isDone);

}

