#include "socketfuncs.h"

namespace Message {

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

	std::vector<unsigned char> Message::recvAll(SOCKET sckt, unsigned int nOfBytesToRecv)
	{	
		// Initializing payload std::vector
		std::vector<unsigned char> payload(nOfBytesToRecv); 

		// receiveData
		int receivedBytes = recv(sckt, (char*)payload.data(), nOfBytesToRecv, 0);

		while (!receivedBytes == nOfBytesToRecv) {
			std::vector<unsigned char> bytesLeft;  
			recv(sckt, (char*)bytesLeft.data(), (nOfBytesToRecv - receivedBytes), 0);
			payload.insert(payload.end(), bytesLeft.begin(), bytesLeft.end());
		}

		return payload;
	}

	std::vector<unsigned char> Message::recvPackets(SOCKET sckt, bool debug)
	{
		std::vector<unsigned char> s_packetLength(4);
		std::vector<unsigned char> s_nOfPacketsLeft(1);
		std::vector<unsigned char> s_typeOfData(1);
		std::vector<unsigned char> receivedData;
		
		recv(sckt, (char*)s_packetLength.data(), 4, 0);
		recv(sckt, (char*)s_nOfPacketsLeft.data(), 1, 0);
		recv(sckt, (char*)s_typeOfData.data(), 1, 0);

		if (debug) { printf("\n[RECEIVED] header %x %x %x %x nOfPacketsLeft %d typeOfData %c\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0], s_typeOfData[0]); }

		int32_t i_packetLength = 0; int8_t i_nOfPacketsLeft = 0;
		memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
		memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
		i_packetLength = ntohl(i_packetLength); int8_t i_nOfPackets = i_nOfPacketsLeft; int totalObjectLength = i_packetLength;

		std::vector<unsigned char> payload = recvAll(sckt, i_packetLength);
		receivedData.insert(receivedData.end(), payload.begin(), payload.end());

		while (i_nOfPacketsLeft > 0) {
			recv(sckt, (char*)s_packetLength.data(), 4, 0);
			recv(sckt, (char*)s_nOfPacketsLeft.data(), 1, 0);
			recv(sckt, (char*)s_typeOfData.data(), 1, 0);

			if (debug) { printf("\n[RECEIVED] header %x %x %x %x nOfPacketsLeft %d typeOfData: %s\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0], s_typeOfData.data()); }

			i_packetLength = 0; i_nOfPacketsLeft = 0;
			memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
			memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
			i_packetLength = ntohl(i_packetLength);


			totalObjectLength += i_packetLength;
			payload = recvAll(sckt, i_packetLength);
			receivedData.insert(receivedData.end(), payload.begin(), payload.end());
		}
		
		switch (s_typeOfData[0]) {
			case 99: {
				receivedData.push_back('\0');
				break;
			}
		}
		
		return receivedData;
	}

	void Message::sendPackets(SOCKET sckt, std::vector<unsigned char> dataToSend, const char* typeOfData, bool debug)
	{
		// Declare & Initialize variables
		int32_t i_sizeOfData = dataToSend.size(); int8_t i_nOfPacketsToSend = (int8_t)ceil((float)i_sizeOfData / (float)PACKET_SIZE);
		int totalBytesSent = 0; 


		// Loop through the payloads
		for (int i = 0; i < i_nOfPacketsToSend; i++) {

			std::vector<unsigned char>::const_iterator startIndex = (dataToSend.begin() + (i * PACKET_SIZE));
			std::vector<unsigned char>::const_iterator endIndex = (dataToSend.begin() + (i * PACKET_SIZE) + min(PACKET_SIZE, dataToSend.size() - totalBytesSent));
			std::vector<unsigned char> payload(startIndex, endIndex);

			int8_t i_nOfPacketsLeft = i_nOfPacketsToSend - (i + 1);
			int32_t i_packetLength = payload.size();

			send(sckt, (char*)&i_packetLength, 4, 0);
			send(sckt, (char*)&i_nOfPacketsLeft, 1, 0);
			send(sckt, typeOfData, 1, 0);
			totalBytesSent += send(sckt, (char*)payload.data(), i_packetLength, 0);

			if (debug) { printf("\n[SENT] header: %x %x %x %x nOfPacketsLeft: %d typeOfData: %s\n", ((unsigned char*)&i_packetLength)[0], ((unsigned char*)&i_packetLength)[1], ((char*)&i_packetLength)[2], ((unsigned char*)&i_packetLength)[3], ((unsigned char*)&i_nOfPacketsLeft)[0], typeOfData); }
		}
		
	}

}
