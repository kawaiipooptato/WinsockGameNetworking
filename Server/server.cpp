#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <thread>

#define BUFFER_SIZE 512
#define PORT 1234
std::string ip = "127.0.0.1";

int main() {
    std::cout << "Starting server..." << std::endl;

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create a UDP socket
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

    // Bind the socket to a local address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str()); /* use INADDR_ANY to bind to any available ip address*/
    serverAddr.sin_port = htons(PORT);

    result = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // When creating a UDP server, you don't need to listen for incoming connections
    // You can just start handling clients immediately
    
    // Container for clients
    std::vector<sockaddr_in> clients;

    // Enter receive loop
    while (true) {
        // Receive a datagram from a client
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        char buffer[BUFFER_SIZE];
        result = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (result == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

		// Console output for debugging
		std::cout << "New message from " << inet_ntoa(clientAddr.sin_addr) << std::endl;

        // Check if client is already in the list
		bool clientExists = false;
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i].sin_addr.s_addr == clientAddr.sin_addr.s_addr) {
				clientExists = true;
				break;
			}
		}
		// If client is not in the list, add it
        if (!clientExists) 
			clients.push_back(clientAddr);
		
		// Send message to all clients
        for (int i = 0; i < clients.size(); i++) {
			result = sendto(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clients[i], sizeof(clients[i]));
            if (result == SOCKET_ERROR) {
				std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
				closesocket(serverSocket);
				WSACleanup();
				return 1;
			}
		}
	}
    
    // Clean up
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}