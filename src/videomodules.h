
#include "socketfuncs.h"
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
#include <stdlib.h>
#include <fstream>
#include <mutex>

class safeString 
{
private:
	std::mutex lock;
	std::string data;

public:
	std::string getData()
	{
		std::lock_guard<std::mutex> lg(lock);
		return data;
	}

	void setData(const std::string& s)
	{
		std::lock_guard<std::mutex> lg(lock);
		data = s;
	}
};

namespace Video 
{

	std::vector<unsigned char> receiveFrameBuffers(const SOCKET& sckt, bool askForNextFrame, bool debug);

	void showFrames(const SOCKET& sckt, bool askForNextFrame, bool debug);

}

