#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <chrono>
#include <thread>

#define BUFFER_SIZE 512
#define PORT 1234
#define TICK_RATE 30 // In Hz
std::string ip = "127.0.0.1";

int main() {
    std::cout << "Client started" << std::endl;

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create a UDP socket
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

    // Set socket to non-blocking mode
    u_long nonBlockingMode = 1;
    if (ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode) != 0) {
        std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Parse initial state from buffer
    // ...
    // Populate local game state with initial state
    // ...

    // Enter game loop
    std::chrono::milliseconds tickInterval(static_cast<int>(1000.0 / TICK_RATE));
    auto nextTick = std::chrono::steady_clock::now() + tickInterval;
    while (true) {
        // Get player input
        // ...
        // Populate buffer with player input
        // ...
        char buffer[BUFFER_SIZE];
        // Send input to server
		result = sendto(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
			std::cerr << "input sendto failed: " << WSAGetLastError() << std::endl;
			/*closesocket(clientSocket);
			WSACleanup();
			return 1;*/
		}

		// Receive updated game state from server
		sockaddr_in fromAddr;
		int fromAddrSize = sizeof(fromAddr);
		result = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&fromAddr, &fromAddrSize);
        if (result == SOCKET_ERROR) {
			std::cerr << "update recvfrom failed: " << WSAGetLastError() << std::endl;
			/*closesocket(clientSocket);
			WSACleanup();
			return 1;*/
		}

        // Parse updated game state from buffer
        // ...
        // Save predicted game state locally
        // ...

        // Wait until the next tick
        std::this_thread::sleep_until(nextTick);
        nextTick += tickInterval;

        // Update predicted game state based on player input
        // ...
        // Note: this should be done on the client side only, without server input
        // ...

        // Compare predicted game state to actual game state     received from server
        // ...
        // Update local game state with actual game state
        // ...
    }

    // Clean up
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
