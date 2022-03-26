#include "socketfuncs.h"

namespace Message {

	SOCKET createSocket(LPCSTR IPAdress, LPCSTR Port, bool makingClient, bool debug)
	{
		// If the function is making a client, it will proceed to do so, else it will proceed to make a server socket
		if (makingClient) {
			int Result = 0;
			WSADATA wsaData;

			Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (Result && debug) { std::cout << "\n[DEBUG] Error " << std::hex << Result << std::endl; return 1; }

			addrinfo* result = NULL, * ptr = NULL, hints;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			Result = getaddrinfo(IPAdress, Port, &hints, &result);
			if (Result && debug) { std::cout << "[DEBUG] Error 0x02" << std::endl; WSACleanup(); return 1; }

			SOCKET sckt = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (sckt == INVALID_SOCKET && debug) {
				std::cout << "[DEBUG] Error at socket(): " << WSAGetLastError() << std::endl;
				freeaddrinfo(result);
				WSACleanup();
				return 1;
			}
			Result = connect(sckt, result->ai_addr, (int)result->ai_addrlen);
			if (Result) { closesocket(sckt); sckt = INVALID_SOCKET; }
			freeaddrinfo(result);
			if (sckt == INVALID_SOCKET && debug) { std::cout << "[DEBUG] Connection timed out" << std::endl; WSACleanup(); return 1; }
			return sckt;
		}
		else {
			WSADATA wsData;
			
			int Result = WSAStartup(MAKEWORD(2, 2), &wsData);
			if (!Result && debug) { std::cout << "\n[DEBUG] Error " << std::hex << Result << std::endl; return 1; }

			SOCKET listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (listenerSocket == INVALID_SOCKET && debug) {
				std::cout << "[DEBUG] Error at socket(): " << WSAGetLastError() << std::endl;
				WSACleanup();
				return 1;
			}

			sockaddr_in hint;
			hint.sin_family = AF_INET;
			hint.sin_port = htons(atoi(Port));
			hint.sin_addr.S_un.S_addr = INADDR_ANY;

			bind(listenerSocket, (sockaddr*)&hint, sizeof(hint));
			if (debug) { std::cout << "[+] Listening for connections... " << std::endl; }
			listen(listenerSocket, SOMAXCONN);

			sockaddr_in client;
			int clientSize = sizeof(client);
			SOCKET server = accept(listenerSocket, (sockaddr*)&client, (int*)&clientSize);

			char host[NI_MAXHOST];
			char port[NI_MAXSERV];

			ZeroMemory(host, NI_MAXHOST);
			ZeroMemory(port, NI_MAXSERV);

			if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, port, NI_MAXSERV, 0) == 0 && debug) {
				std::cout << "[+] Host: " << host << " connected on port " << port << std::endl;
			}
			else {
				std::cout << "[+] Host: " << host << " connected on port " << ntohs(client.sin_port) << std::endl;
			}

			closesocket(listenerSocket);
			return server;
		}
	}

	// A "safer" and more convinient implementation of the standard recv() function
	std::vector<unsigned char> Message::recvAll(SOCKET sckt, unsigned int nOfBytesToRecv)
	{	
		// Initializing payload's vector
		std::vector<unsigned char> payload(nOfBytesToRecv); 

		// Receive the number of bytes requested by the caller
		int receivedBytes = recv(sckt, (char*)payload.data(), nOfBytesToRecv, 0);

		// Check for lost bytes, if some lost bytes are found, they will be appended to the end of the payload
		while (!receivedBytes == nOfBytesToRecv) {
			std::vector<unsigned char> bytesLeft;  
			recv(sckt, (char*)bytesLeft.data(), (nOfBytesToRecv - receivedBytes), 0);
			payload.insert(payload.end(), bytesLeft.begin(), bytesLeft.end());
		}

		return payload;
	}

	//                                                                              number of packets left to receive
	// Implementation of the recvAll() function packet system --> packetHeader: 0x0 0x0 0x0 0x0 | 0x0 0x0 | 0x0
	//                                                                                 ^       |             ^
	//                                                                                 |       |             |
	//                                                                           packet length | type of request (0x63 "c", 0x76 "v")
	std::vector<unsigned char> Message::recvPackets(SOCKET sckt, bool debug)
	{
		// Initializing the vectors that will contain the packet header data
		std::vector<unsigned char> s_packetLength(4);
		std::vector<unsigned char> s_nOfPacketsLeft(2);
		std::vector<unsigned char> s_typeOfRequest(1);
		std::vector<unsigned char> receivedData;
		
		// Loading the data into the vectors
		s_packetLength = recvAll(sckt, 4);
		s_nOfPacketsLeft = recvAll(sckt, 2);
		s_typeOfRequest = recvAll(sckt, 1);

		if (debug) { printf("[RECEIVED] Header: packetLength %x %x %x %x | nOfPacketsLeft %x %x | typeOfRequest %x\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0], s_nOfPacketsLeft[1], s_typeOfRequest[0]); }
		
		// Copy the data into local variables
		int32_t i_packetLength = 0; int16_t i_nOfPacketsLeft = 0;
		memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
		memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
		i_packetLength = ntohl(i_packetLength); int16_t i_nOfPackets = i_nOfPacketsLeft; int totalObjectLength = i_packetLength;

		std::vector<unsigned char> payload = recvAll(sckt, i_packetLength);
		receivedData.insert(receivedData.end(), payload.begin(), payload.end());

		// If there are more packets left, keep receiving the data and reconstruct the original message
		while (i_nOfPacketsLeft > 0) {
			s_packetLength = recvAll(sckt, 4);
			s_nOfPacketsLeft = recvAll(sckt, 2);
			s_typeOfRequest = recvAll(sckt, 1);

			if (debug) { printf("[RECEIVED] Header: packetLength %x %x %x %x | nOfPacketsLeft %x %x | typeOfRequest %x\n", s_packetLength[0], s_packetLength[1], s_packetLength[2], s_packetLength[3], s_nOfPacketsLeft[0], s_nOfPacketsLeft[1], s_typeOfRequest[0]); }

			i_packetLength = 0; i_nOfPacketsLeft = 0;
			memcpy(&i_packetLength, s_packetLength.data(), sizeof(i_packetLength));
			memcpy(&i_nOfPacketsLeft, s_nOfPacketsLeft.data(), sizeof(i_nOfPacketsLeft));
			i_packetLength = ntohl(i_packetLength);


			totalObjectLength += i_packetLength;
			payload = recvAll(sckt, i_packetLength);
			receivedData.insert(receivedData.end(), payload.begin(), payload.end());
		}
		
		// Look at the type of request we are receiving, if it is 99 (0x63 or ASCII for "c") it means we are receiving string data so we need to push a null byte at the end of the message
		switch (s_typeOfRequest[0]) {
			case 99: {
				receivedData.push_back('\0');
				break;
			}
		}
		
		return receivedData;
	}

	// Implementation of the send() function with the packet system 
	void Message::sendPackets(SOCKET sckt, std::vector<unsigned char> dataToSend, const char* typeOfRequest, bool debug)
	{
		// Initialize control variables
		int32_t i_sizeOfData = dataToSend.size(); 
		int16_t i_nOfPacketsToSend = (int16_t)ceil((float)i_sizeOfData / (float)PACKET_SIZE);
		int totalBytesSent = 0; 


		// Loop through the packets
		for (short i = 0; i < i_nOfPacketsToSend; i++) {

			// Divide the data to send into n packets of PACKET_SIZE size
			std::vector<unsigned char>::const_iterator startIndex = (dataToSend.begin() + (i * PACKET_SIZE));
			std::vector<unsigned char>::const_iterator endIndex = (dataToSend.begin() + (i * PACKET_SIZE) + min(PACKET_SIZE, dataToSend.size() - totalBytesSent));
			std::vector<unsigned char> payload(startIndex, endIndex);

			int16_t i_nOfPacketsLeft = i_nOfPacketsToSend - (i + 1);
			int32_t i_packetLength = payload.size();

			// Send the packet header first, then the payload, while keeping track of the number of bytes sent
			send(sckt, (char*)&i_packetLength, 4, 0);
			send(sckt, (char*)&i_nOfPacketsLeft, 2, 0);
			send(sckt, typeOfRequest, 1, 0);
			totalBytesSent += send(sckt, (char*)payload.data(), i_packetLength, 0);

			if (debug) { printf("[SENT] Header: packetLength %x %x %x %x | nOfPacketsLeft %x %x | typeOfRequest %x\n", ((unsigned char*)&i_packetLength)[0], ((unsigned char*)&i_packetLength)[1], ((char*)&i_packetLength)[2], ((unsigned char*)&i_packetLength)[3], ((unsigned char*)&i_nOfPacketsLeft)[0], ((unsigned char*)&i_nOfPacketsLeft)[1], typeOfRequest[0]); }
		}
		
	}

}
