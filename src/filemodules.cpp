#include "filemodules.h"

bool File::checkIfValidPath(std::string dir)
{	
	std::ifstream checkIfValidDirectory(dir);
	if (dir.length()<2) {
		return false;
	}
	else if (!checkIfValidDirectory) {
		return false;
	}
	return true;
}

bool File::correctGetCommandSyntax(std::string command)
{
	if (command.length() == 8 || command.length() == 7) {
		std::cout << "SYNTAX: " << std::endl
			<< "  getfile [-options] [filename] [savedir]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: add a directory where the file will be put" << std::endl
			<< "     -p: add a persistent directory where all the files will be put in" << std::endl;
		return false;
	}
	else if ((const char*)command.at(8) == "-") {
		auto lastSpace = command.find_last_of(" ", 0);
		if ((const char*)command.at(9) != "s" || (const char*)command.at(9) != "p") {
			std::cout << "SYNTAX: " << std::endl
				<< "  getfile [-options] [filename] [savedir]" << std::endl << std::endl
				<< "  Options:" << std::endl
				<< "     -s: add a directory where the file will be put" << std::endl
				<< "     -p: add a persistent directory where all the files will be put in" << std::endl;
			return false;
		}
		else if (!checkIfValidPath(command.substr(lastSpace, command.length() - lastSpace + 1))) {
			std::cout << "SYNTAX: " << std::endl
				<< "  getfile [-options] [filename] [savedir]" << std::endl << std::endl
				<< "  Options:" << std::endl
				<< "     -s: add a directory where the file will be put" << std::endl
				<< "     -p: add a persistent directory where all the files will be put in" << std::endl;
				return false;
		}
	}
	else if (command.find(".") == std::string::npos) {
		std::cout << "SYNTAX: " << std::endl
			<< "  getfile [-options] [filename] [savedir]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: add a directory where the file will be put" << std::endl
			<< "     -p: add a persistent directory where all the files will be put in" << std::endl;
		return false;
	}

	return true;
}

int File::getFile(const SOCKET& sckt, std::string command, bool debug)
{	
	while ((const char*)command.at(command.length() - 1) == " ") {
		command.pop_back();
	}
	if (!correctGetCommandSyntax(command)) {
		return -1;
	}

	std::string savePath = "";
	std::string fileName = "";
	size_t lastSpace = command.find_last_of(" ", command.length());

	if ((const char*)command.at(8) == "-") {
		if ((const char*)command.at(9) == "s") {
			savePath = command.substr(lastSpace, command.length() - lastSpace + 1) + "\\";
		}
		else if ((const char*)command.at(9) == "p") {
			savePath = command.substr(lastSpace, command.length() - lastSpace + 1) + "\\";
			// TODO: add support for memorization of cross-session data
		}
		fileName = command.substr(12, lastSpace - 12);
	}
	else {
		fileName = command.substr(8, command.length() - 8);
	}

	std::string finalCommand = "getfile " + fileName;
	std::cout << finalCommand;
	std::vector<unsigned char> inputCommand(finalCommand.begin(), finalCommand.end());
	Message::sendMsg(sckt, inputCommand, "f", debug);

	std::vector<unsigned char> fileData = Message::recvMsg(sckt, debug);
	if (fileData[0] == 0x45 && fileData[1] == 0x45) {
		fileData.erase(fileData.begin());
		fileData.erase(fileData.begin());
		std::cout << fileData.data();
		return -1;
	}
	else {
		std::string fileNameAndPath = savePath + fileName;
		std::ofstream fileStream(fileNameAndPath, std::ios::out | std::ios::binary);

		if (fileStream.is_open())
		{
			fileStream.write((char*)fileData.data(), fileData.size());
			fileStream.close();
		}
		return 1;
	}
}

int File::sendFile(const SOCKET& sckt, std::string command, bool debug)
{
	return 0;
}
