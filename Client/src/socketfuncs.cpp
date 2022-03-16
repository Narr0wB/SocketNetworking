#include "socketfuncs.h"

namespace Message {
	
	char* toBytes(void* src) {
		return (char*)src;
	}

	SOCKET createSocket(LPCSTR IPAdress, LPCSTR Port)
	{
		int Result = 0;
		WSADATA wsaData;

		Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (Result) { printf("\n[DEBUG] ERRORE %d", Result); return 1; }

		addrinfo* result = NULL, * ptr = NULL, hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		Result = getaddrinfo(IPAdress, Port, &hints, &result);
		if (Result) { printf("ERRORE 2"); WSACleanup(); return 1; }

		SOCKET sckt = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sckt == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}
		Result = connect(sckt, result->ai_addr, (int)result->ai_addrlen);
		if (Result) { closesocket(sckt); sckt = INVALID_SOCKET; }
		freeaddrinfo(result);
		if (sckt == INVALID_SOCKET) { printf("non e' stato possibile connettersi"); WSACleanup(); return 1; }
		return sckt;
	}

	std::vector<char> Message::recvAll(SOCKET sckt,int nOfBytesToRecv)
	{	
		// Initializing packet and data std::vector
		std::vector<char> dataPacket; 
		
		// Initializing control variables
		int chunk_size = PACKET_SIZE; if (nOfBytesToRecv < chunk_size) { chunk_size = nOfBytesToRecv; }
		int receivedTotalBytes = 0;
		std::vector<char> chunkOfPacket(chunk_size);
		std::string tempString;

		
		// Main loop
		while (receivedTotalBytes < (nOfBytesToRecv -1)) {
			int receivedBytes = recv(sckt, chunkOfPacket.data(), min(nOfBytesToRecv - receivedTotalBytes, chunk_size), 0);
			tempString += std::string(chunkOfPacket.data());
			receivedTotalBytes += receivedBytes;
		}

		std::copy(tempString.begin(), tempString.end(), std::back_inserter(dataPacket));
		while (dataPacket.size() > nOfBytesToRecv) { dataPacket.pop_back(); }

		return dataPacket;
	}

	std::vector<char> Message::recvPackets(SOCKET sckt, bool debug, bool receivingString)
	{
		std::vector<char> s_packetLength(4);
		std::vector<char> s_nOfPacketsLeft(1);
		std::vector<char> receivedData;
		
		int bytesreceived = recv(sckt, s_packetLength.data(), 4, 0);
		bytesreceived = recv(sckt, s_nOfPacketsLeft.data(), 1, 0);

		if (debug) { printf("\nheader %x %x %x %x nOfPacketsLeft %d\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0]); }

		int32_t i_packetLength = 0; int8_t i_nOfPacketsLeft = 0;
		memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
		memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
		i_packetLength = ntohl(i_packetLength); int8_t i_nOfPackets = i_nOfPacketsLeft; int totalObjectLength = i_packetLength;

		std::vector<char> payload = recvAll(sckt, i_packetLength);
		receivedData.insert(receivedData.end(), payload.begin(), payload.end());
		
		while (i_nOfPacketsLeft > 0) {
			bytesreceived = recv(sckt, s_packetLength.data(), 4, 0);
			bytesreceived = recv(sckt, s_nOfPacketsLeft.data(), 1, 0);

			if (debug) { printf("\nheader %x %x %x %x nOfPacketsLeft %d\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0]); }

			i_packetLength = 0; i_nOfPacketsLeft = 0;
			memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
			memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
			i_packetLength = ntohl(i_packetLength);


			totalObjectLength += i_packetLength;
			payload = recvAll(sckt, i_packetLength);
			receivedData.insert(receivedData.end(), payload.begin(), payload.end());
		}

		if (receivingString) { receivedData.push_back('\0'); }
		
		return receivedData;
	}

	void Message::sendPackets(SOCKET sckt, char* command, bool debug)
	{
		// Declare & Initialize variables
		int32_t i_lengthOfCommand = strlen(command); int8_t i_numberOfPackets = (int8_t)ceil((float)i_lengthOfCommand / (float)PACKET_SIZE);
		char *s_lengthOfCommand = toBytes(&i_lengthOfCommand);
		int bytesPacketSent = 0; 
		std::string strCommand(command);

		// Loop through the packets
		for (int i = 0; i < i_numberOfPackets; i++) {
			int8_t i_numberOfPacketsLeft = i_numberOfPackets - (i+1);
			char* s_numberOfPacketsLeft = toBytes(&i_numberOfPacketsLeft);
			std::string substr = strCommand.substr(i * PACKET_SIZE, min(PACKET_SIZE, strCommand.length() - bytesPacketSent));
			int32_t i_lengthOfPacket = substr.length(); char *s_lengthOfPacket = toBytes(&i_lengthOfPacket);
			int bytesSent = send(sckt, s_lengthOfPacket, 4, 0);
			bytesSent = send(sckt, s_numberOfPacketsLeft, 1, 0);
			bytesPacketSent += send(sckt, substr.c_str(), substr.length(), 0); 
			if (debug) { printf("\nheader: %x %x %x %x nOfPacketsLeft: %d Payload: %s\n", s_lengthOfPacket[0], s_lengthOfPacket[1], s_lengthOfPacket[2], s_lengthOfPacket[3], s_numberOfPacketsLeft[0], substr.c_str()); }
		}
		
	}

}
namespace Video {

}