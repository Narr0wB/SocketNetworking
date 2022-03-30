#include <stdio.h>
#include <iostream>
#include <thread>
#include <fstream>
#include "socketfuncs.h"

namespace File {
	bool checkIfValidPath(std::string dir);

	bool correctGetCommandSyntax(std::string command);

	int getFile(const SOCKET& sckt, std::string command, bool debug);

	int sendFile(const SOCKET& sckt, std::string command, bool debug);
}

