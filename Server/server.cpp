#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <chrono>

#define BUFFER_SIZE 512
#define PORT 1234
std::string ip = "127.0.0.1";
int timeoutInSeconds = 60;

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
    std::vector<std::pair<sockaddr_in, std::chrono::time_point<std::chrono::system_clock>>> clients;

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

        // Check if client is already in the list
		bool clientExists = false;
        for (auto& client : clients) {
            if (client.first.sin_addr.s_addr == clientAddr.sin_addr.s_addr) {
				clientExists = true;
				break;
			}
		}

		// If client is not in the list, add it
        if (!clientExists)
			clients.push_back(std::make_pair(clientAddr, std::chrono::system_clock::now()));

		// Send a response to the client
		result = sendto(serverSocket, buffer, result, 0, (sockaddr*)&clientAddr, clientAddrSize);
        if (result == SOCKET_ERROR) {
			std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		// Remove clients that have not sent a message in the last 60 seconds
        for (auto it = clients.begin(); it != clients.end();) {
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - it->second).count() > timeoutInSeconds)
				it = clients.erase(it);
            else 
				++it;
		}
	}

	// Clean up
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
    