// src/main.cpp
#include "Core/Game.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    bool startAsServer = false;
    std::string serverIp = "127.0.0.1"; // Default for client, or if server hosts locally
    unsigned short serverPort = 12345;    // Default port

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server") {
            startAsServer = true;
            std::cout << "Starting as server..." << std::endl;
        } else if (arg == "--connect" && i + 1 < argc) {
            serverIp = argv[++i];
            std::cout << "Will attempt to connect to server at: " << serverIp << std::endl;
        } else if (arg == "--port" && i + 1 < argc) {
            try {
                serverPort = static_cast<unsigned short>(std::stoi(argv[++i]));
                std::cout << "Using port: " << serverPort << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Invalid port number: " << argv[i-1] << ". Using default " << serverPort << std::endl;
            }
        }
    }

    RTSEngine::Core::Game game(startAsServer, serverIp, serverPort);
    return game.run();
}