#include <stdio.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include "socketfuncs.h"

#define MAX_FILE_SIZE 2000000000

namespace File 
{
	std::string getCurrentDir(const SOCKET& sckt);

	bool correctGetCommandSyntax(std::string command);

	bool correctSendCommandSyntax(std::string command);

	int getFile(const SOCKET& sckt, std::string command, bool debug);

	int sendFile(const SOCKET& sckt, std::string command, bool debug);
}

