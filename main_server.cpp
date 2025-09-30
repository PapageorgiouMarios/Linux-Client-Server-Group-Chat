#include <iostream>
#include <thread>
#include <chrono>
#include "server.h"

int main(int argc, char* argv[]) 
{
    
    if (argc < 2) 
    {
        std::cerr << "✗ Invalid arguments" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    Server server(port);
    std::thread serverThread([&server]() { server.start(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (server.running) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Main thread sleeps to reduce CPU usage
    }

    server.stop();
    if (serverThread.joinable()) serverThread.join();
    return 0;
}
