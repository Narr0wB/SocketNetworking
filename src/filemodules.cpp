#include "filemodules.h"


std::string File::getCurrentDir(const SOCKET& sckt)
{
	auto currentDirectory = Message::recvMsg(sckt, false);
	return std::string(currentDirectory.begin(), currentDirectory.end());
}

// command: getfile [filename | filepath] [savepath]
bool File::correctGetCommandSyntax(std::string command)
{
	// Since a file path is necessary, im looking for the last colon as it would identify the save path given that the save path is last in the command syntax
	size_t lastColon = command.find_last_of(":");
	size_t nOfSpaces = 1;
	size_t iterator = command.find(".");

	while (iterator < command.length()-1)
	{
		iterator++;
		if (command.at(iterator) == ' ')
		{
			nOfSpaces++;
			break;
		}

	}

	// If there is no save path
	if (nOfSpaces == 1 || lastColon == std::string::npos)
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
	// If there is only a file path but missing a save path
	return true;
}

// command: sendfile [filepath] [OPTIONAL: filedestinationpath] 
bool File::correctSendCommandSyntax(std::string command)
{
	// Find filepath's colon
	size_t lastColon = command.find_last_of(":");

	// If there isnt a filepath
	if (lastColon == std::string::npos || command.find(".") == std::string::npos)
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [filepath] [OPTIONAL: filedestinationpath]" << std::endl << std::endl;
		return false;
	}
	// If the command is too short
	if (command.length() == 8 || command.length() == 7)
	{
		std::cout << "SYNTAX: " << std::endl
			<< "  sendfile [filepath] [OPTIONAL: filedestinationpath]" << std::endl << std::endl;
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

	// Check if the syntax of the command is correct
	if (!correctGetCommandSyntax(command)) return -1;

	std::string savePath;
	std::string fileName;
	size_t startOfPath = command.find_last_of(":") - 1;

	// Get whatever is between "getfile " and save path, it will later check if its either a path or just a file name
	fileName = command.substr(8, startOfPath-9);
	savePath = command.substr(startOfPath, command.length() - startOfPath + 1) + "\\"; // Get save path

	// Make the command the program will send to the server, convert it to unsigned char, and send it as a file request
	std::string finalCommand = "getfile " + fileName;
	std::vector<unsigned char> inputCommand(finalCommand.begin(), finalCommand.end());
	Message::sendMsg(sckt, inputCommand, "f", debug);


	// Get response from the server
	std::vector<unsigned char> fileData = Message::recvMsg(sckt, debug);

	// Error handling, if the server didnt find filename or filepath or the file is bigger than 2GB
	if (fileData[0] == 0x45 && fileData[1] == 0x45) 
	{
		fileData.erase(fileData.begin(), fileData.begin()+2);
		std::cout << fileData.data();
		return -1;
	}

	// Check if whatever was between the getfile command and save path was a path, if so extract the file name as we will need it to write the file in the new location 
	if (fileName.find(":") != std::string::npos) 
	{
		size_t lastBackSlash = fileName.find_last_of("\\");
		fileName = fileName.substr(lastBackSlash+1, fileName.length() - lastBackSlash);
	}

	// Make the final save path with the filename
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
		return -1;
	}

	return 1;
}

int File::sendFile(const SOCKET& sckt, std::string command, bool debug)
{
	// Get rid of any whitespaces at the end of the command
	while (command.at(command.length() - 1) == 0x20)
	{
		command.pop_back();
	}

	// Check syntax
	if (!correctSendCommandSyntax(command)) return -1;

	std::string filePath;
	std::string fileName;
	std::string finalCommand;
	size_t firstColon = command.find_first_of(":");
	size_t lastColon = command.find_last_of(":");

	// Extract the file path
	if (firstColon == lastColon) filePath = command.substr(9);
	else filePath = command.substr(9, lastColon - 11);

	// Extract the file name
	size_t lastBackSlash = filePath.find_last_of("\\");
	fileName = filePath.substr(lastBackSlash+1, filePath.length() - lastBackSlash);

	// Create the file stream
	std::ifstream fileStream(filePath, std::ios::binary | std::ios::in | std::ios::ate);

	// Check if the file path is valid
	if (!fileStream.is_open()) 
	{
		std::cout << "Invalid file path!" << std::endl;
		return -1;
	}

	// Get file size and check if its bigger than 2GB
	auto fileSize = fileStream.tellg();
	if (fileSize > MAX_FILE_SIZE) { std::cout << "File's too big";  return -1; }

	// Create the file buffer to send
	std::vector<unsigned char> fileBuffer(fileSize);

	// Read the file and load the buffer
	fileStream.seekg(0, std::ios::beg);
	fileStream.read((char*)fileBuffer.data(), fileSize);
	fileStream.close();

	// Make the command the program will send to the server, convert it to unsigned char, and send it as a file request
	if (firstColon == lastColon) finalCommand = "sendfile " + fileName;
	else finalCommand = "sendfile " + fileName + " " + command.substr(lastColon-1);
	std::vector<unsigned char> inputCommand(finalCommand.begin(), finalCommand.end());
	Message::sendMsg(sckt, inputCommand, "f", debug);

	// Send the file data
	Message::sendMsg(sckt, fileBuffer, "f", debug);

	return 1;
}
