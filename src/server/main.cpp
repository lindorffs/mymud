// src/main.cpp
#include "Server.h"
#include <iostream>
#include <string>
#include <vector>

#define SERVER

int main(int argc, char* argv[]) {
    std::string serverIp = "0.0.0.0"; // Default for client, or if server hosts locally
    unsigned short serverPort = 12345;    // Default port

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
		if (arg == "--port" && i + 1 < argc) {
            try {
                serverPort = static_cast<unsigned short>(std::stoi(argv[++i]));
                std::cout << "Using port: " << serverPort << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Invalid port number: " << argv[i-1] << ". Using default " << serverPort << std::endl;
            }
        }
    }

    RTSEngine::Core::Server server(serverIp, serverPort);
    return server.run();
}
