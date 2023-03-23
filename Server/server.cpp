#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <stdexcept>

#define BUFFER_SIZE 512
#define PORT 1234
std::string ip = "127.0.0.1";
int timeoutInSeconds = 60;
SOCKET serverSocket;

struct GameState {
    int playerX;
    int playerY;
    // add any other game state variables here
};

void printLastError(const std::string& message) {
    int error = WSAGetLastError();
    std::cerr << message << " Error code: " << error << std::endl;
}

int main() {
    std::cout << "Starting server..." << std::endl;

    try {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        // Create a UDP socket
        serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET) {
            throw std::runtime_error("socket failed");
        }

        // Bind the socket to a local address and port
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str()); /* use INADDR_ANY to bind to any available ip address*/
        serverAddr.sin_port = htons(PORT);

        result = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            throw std::runtime_error("bind failed");
        }

        // When creating a UDP server, you don't need to listen for incoming connections
        // You can just start handling clients immediately

        // Container for clients
        std::vector<std::pair<sockaddr_in, std::chrono::time_point<std::chrono::system_clock>>> clients;

        // Initialize game state
        GameState gameState;
        gameState.playerX = 0;
        gameState.playerY = 0;
        // add any other game state variables here

        // Enter receive loop
        while (true) {
            // Receive a datagram from a client
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            char buffer[BUFFER_SIZE];
            result = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
            if (result == SOCKET_ERROR) {
                printLastError("recvfrom failed");
                throw std::runtime_error("recvfrom failed");
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

            // Deserialize the received data into a message
            int receivedPlayerX, receivedPlayerY;
            std::memcpy(&receivedPlayerX, &buffer[0], sizeof(receivedPlayerX));
            std::memcpy(&receivedPlayerY, &buffer[sizeof(receivedPlayerX)], sizeof(receivedPlayerY));

            // Server reconciliation
            if (receivedPlayerX != gameState.playerX || receivedPlayerY != gameState.playerY)
            {
                // If the client's position is different from the server's, update the server's position
                gameState.playerX = receivedPlayerX;
                gameState.playerY = receivedPlayerY;
                // add any other game state reconciliation here
            }

            // Send game state back to client
            std::memcpy(&buffer[0], &gameState.playerX, sizeof(gameState.playerX));
            std::memcpy(&buffer[sizeof(gameState.playerX)], &gameState.playerY, sizeof(gameState.playerY));
            result = sendto(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, clientAddrSize);
            if (result == SOCKET_ERROR) {
                printLastError("sendto failed");
                throw std::runtime_error("sendto failed");
            }

            // Check for clients that have timed out
            auto currentTime = std::chrono::system_clock::now();
            for (auto it = clients.begin(); it != clients.end();) {
                if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - it->second).count() >= timeoutInSeconds) {
                    std::cout << "Client timed out: " << inet_ntoa(it->first.sin_addr) << std::endl;
                    it = clients.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        // Clean up
        closesocket(serverSocket);
        WSACleanup();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}