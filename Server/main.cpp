#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <thread>

#define BUFFER_SIZE 512
#define PORT 1234
std::string ip = "127.0.0.1";

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int result;

    // Send initial state to client
    // ...
    // Populate buffer with initial state
    // ...
    result = send(clientSocket, buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return;
    }

    // Enter game loop
    while (true) {
        // Receive input from client
        result = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return;
        }

        // Update game state based on client input
        // ...

        // Update game state based on server logic
        // ...

        // Send updated game state to client
        // ...
        // Populate buffer with updated game state
        // ...
        result = send(clientSocket, buffer, BUFFER_SIZE, 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "send failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return;
        }
    }

    // Clean up
    closesocket(clientSocket);
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    // Listen for incoming connections
    result = listen(serverSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::vector<std::thread> threads;

    // Enter accept loop
    while (true) {
        // Accept a connection from a client
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        // Console output for debugging
        std::cout << "Client connected from " << inet_ntoa(serverAddr.sin_addr) << std::endl;

        // Spawn a thread to handle the client
        threads.push_back(std::thread(handleClient, clientSocket));
    }

    // Clean up
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}