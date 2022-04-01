#include "filemodules.h"


std::string File::getCurrentDir(const SOCKET& sckt)
{
	auto currentDirectory = Message::recvMsg(sckt, false);
	return std::string(currentDirectory.begin(), currentDirectory.end());
}

// Check for the correct syntax of the get command
bool File::correctGetCommandSyntax(std::string command)
{
	// Since a savePath is necessary, im looking for a colon as it means generally a complete path
	size_t lastColon = command.find_last_of(":");

	// If there is no savePath
	if (lastColon == std::string::npos)
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  getfile [filename | filepath] [savepath]" << std::endl << std::endl;
		return false;
	}
	// If the command is too short
	if (command.length() == 8 || command.length() == 7) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  getfile [filename | filepath] [savepath]" << std::endl << std::endl;
		return false;
	}
	// If there isnt a filename or filepath
	if (command.find(".") == std::string::npos) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  getfile [filename | filepath] [savepath]" << std::endl << std::endl;
		return false;
	}
	return true;
}

// command: sendfile [options] [filepath] [filedestinationpath]
// options: 
//		-s: to add a usual file destination (for the session)
bool File::correctSendCommandSyntax(std::string command)
{
	size_t lastColon = command.find_last_of(":");
	size_t firstColon = command.find_first_of(":");
	if (firstColon == lastColon) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [options] [filepath] [filedestinationpath]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: to add a usual file destination (for the session)" << std::endl;
		return false;
	}
	if (command.length() == 8 || command.length() == 9) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [options] [filepath] [filedestinationpath]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: to add a usual file destination (for the session)" << std::endl;
		return false;
	}
	if (command.find(".") == std::string::npos) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [options] [filepath] [filedestinationpath]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: to add a usual file destination (for the session)" << std::endl;
		return false;
	}
	if (command.at(9) == 0x20 && command.at(10) != 0x73) 
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [options] [filepath] [filedestinationpath]" << std::endl << std::endl
			<< "  Options:" << std::endl
			<< "     -s: to add a usual file destination (for the session)" << std::endl;
		return false;
	}
	return true;
}

int File::getFile(const SOCKET& sckt, std::string command, bool debug)
{	
	// Get rid of any whitespaces at the end of the command
	while (command.at(command.length() - 1) == 0x20) 
	{
		command.pop_back();
	}

	// check if the syntax of the command is correct
	if (!correctGetCommandSyntax(command)) return -1;

	std::string savePath;
	std::string fileName;
	size_t startOfPath = command.find_last_of(":") - 1;

	// Get whatever is between "getfile " and savepath, it will later check if this is either a path or just a file name
	fileName = command.substr(8, startOfPath-9);
	savePath = command.substr(startOfPath, command.length() - startOfPath + 1) + "\\"; // Get savepath

	// Make the command the program will send to the server, convert it to unsigned char, and send it
	std::string finalCommand = "getfile " + fileName;
	std::vector<unsigned char> inputCommand(finalCommand.begin(), finalCommand.end());
	Message::sendMsg(sckt, inputCommand, "f", debug);

	// Get response from the server
	std::vector<unsigned char> fileData = Message::recvMsg(sckt, debug);

	// Error handling, if the server didnt find filename or filepath
	if (fileData[0] == 0x45 && fileData[1] == 0x45) 
	{
		fileData.erase(fileData.begin()+2);
		std::cout << fileData.data();
		return -1;
	}

	// Check if whatever was between getfile and savepath was a path, if so extract the file name as we will need it to write the file
	if (fileName.find(":") != std::string::npos) 
	{
		size_t lastBackSlash = fileName.find_last_of("\\");
		fileName = fileName.substr(lastBackSlash, fileName.length() - lastBackSlash);
	}

	// Make the final savepath with the filename
	std::string fileNameAndPath = savePath + fileName;

	// Create the output file stream
	std::ofstream fileStream(fileNameAndPath, std::ios::out | std::ios::binary);

	// If the stream is open
	if (fileStream.is_open())
	{
		fileStream.write((char*)fileData.data(), fileData.size());
		fileStream.close();
	}
	else
	{
		std::cout << "Invalid save path!" << std::endl;
	}

	// Print the current dir
	std::cout << getCurrentDir(sckt);
	return 1;
}

int File::sendFile(const SOCKET& sckt, std::string command, bool debug)
{
	if (!correctSendCommandSyntax(command)) return -1;
	return 1;
}
