#include <iostream>
#include <cstdlib> 
#include <string>
#include <thread>
#include <chrono>
#include "client.h"

int main() 
{
    const char* host = std::getenv("SERVER_HOST");
    const char* port_str = std::getenv("SERVER_PORT");

    if (host == nullptr || port_str == nullptr) 
    {
        std::cerr << "⚠ SERVER_HOST or SERVER_PORT not set in environment variables." << std::endl;
        return 1;
    }

    std::string host_str(host);
    int port = std::stoi(port_str);

    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Wait some time before connection

    while(name.empty()) 
    {
        std::cout << "⚠ Name cannot be empty" << std::endl;
        std::cout << "Enter your name: ";
        std::getline(std::cin, name);
    }

    Client client(host_str, port, name);
    if (!client.connectToServer()) 
    {
        std::cerr << "✗ Unable to connect to " << host_str << ":" << port << std::endl;
        return 1;
    }
    else
    {
        std::cout << "✓ Connected as '" << name << "'. Type messages and press Enter to send" << std::endl;
        std::cout << "Hint: Type 'quit' + Enter to disconnect and exit." << std::endl;
    }

    std::string line;
    while (client.isConnected()) 
    {
        if (!std::getline(std::cin, line)) {
            std::cout << "⚠ Disconnecting..." << std::endl;
            client.disconnect();
            break;
        }

        if (line.empty()) 
        {
            continue;
        }

        if (line == "quit") 
        {
            std::cout << "✓ Exiting..." << std::endl;
            client.disconnect();
            break;
        }

        if (!client.sendMessage(line)) 
        {
            std::cerr << "✗ Failed to send (connection may be closed)" << std::endl;
            std::cout << "⚠ Disconnecting..." << std::endl;
            client.disconnect();
            break;
        }

        std::cout << "You: " << line << std::endl;
    }

    client.disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}
