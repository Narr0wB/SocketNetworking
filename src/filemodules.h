#include <stdio.h>
#include <iostream>
#include <thread>
#include <fstream>
#include "socketfuncs.h"

namespace File 
{
	std::string getCurrentDir(const SOCKET& sckt);

	bool correctGetCommandSyntax(std::string command);

	bool correctSendCommandSyntax(std::string command);

	int getFile(const SOCKET& sckt, std::string command, bool debug);

	int sendFile(const SOCKET& sckt, std::string command, bool debug);
}

