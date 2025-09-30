#include "server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int create_test_socket(const std::string& host, int port) 
{
    int sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock < 0) return -1; // Socket creation failed

    sockaddr_in serverAddr{}; // Server address structure
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(port); // Port number
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr); // Convert IP address

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) //  Connect to server
    {
        close(sock);
        return -1;
    }

    return sock;
}

int main() 
{
    std::cout << "=== Server Test Suite ===" << std::endl;
    Server server(9999);

    // Launch server in its own thread
    std::thread serverThread([&server]() { server.start(); });
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // ---- Test 1: Start/Stop lifecycle ----
    std::cout << "==================================" << std::endl;
    std::cout << "1) Testing server start" << std::endl;
    if (server.get_connection_count() == 0) 
        std::cout << "✓ Server started (0 sockets connected)" << std::endl;
    else
        std::cout << "✗ Unexpected socket count after start: " << server.get_connection_count() << std::endl;
    std::cout << "==================================\n" << std::endl;

    // ---- Test 2: Single socket connection ----
    std::cout << "===========================================" << std::endl;
    std::cout << "2) Testing single socket connection" << std::endl;
    int clientSock1 = create_test_socket("0.0.0.0", 9999);
    if (clientSock1 >= 0) 
        std::cout << "✓ Socket 1 connected" << std::endl;
    else 
        std::cout << "✗ Socket 1 failed to connect" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server.get_connection_count() >= 1)
        std::cout << "✓ Server reports " << server.get_connection_count() << " socket(s)" << std::endl;
    else
        std::cout << "✗ Server did not register Socket 1" << std::endl;
    std::cout << "===========================================\n" << std::endl;

    // ---- Test 3: Multiple sockets ----
    std::cout << "==============================================" << std::endl;
    std::cout << "3) Testing multiple sockets connection" << std::endl;
    int clientSock2 = create_test_socket("0.0.0.0", 9999);
    if (clientSock2 >= 0) 
        std::cout << "✓ Client Socket 2 connected" << std::endl;
    else 
        std::cout << "✗ Client Socket 2 failed to connect" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server.get_connection_count() >= 2)
        std::cout << "✓ Server reports " << server.get_connection_count() << " sockets" << std::endl;
    else
        std::cout << "✗ Server did not register sockets" << std::endl;
    std::cout << "==============================================\n" << std::endl;

    // ---- Test 4: Broadcast message ----
    std::cout << "===================================================" << std::endl;
    std::cout << "4) Testing broadcast (Socket 1 -> Socket 2)" << std::endl;
    if (clientSock1 >= 0 && clientSock2 >= 0) 
    {
        std::string msg = "Hello from Socket 1\n";
        if (send(clientSock1, msg.c_str(), msg.size(), 0) > 0) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            char buffer[4096];
            ssize_t received = recv(clientSock2, buffer, sizeof(buffer)-1, 0);

            if (received > 0) 
            {
                buffer[received] = '\0';
                std::string recvStr(buffer);
                std::cout << "Socket 2 received: " << recvStr;
                if (recvStr.find("Hello from Socket 1") != std::string::npos)
                    std::cout << "✓ Broadcast successful" << std::endl;
                else
                    std::cout << "✗ Broadcast content mismatch" << std::endl;
            }
            else
                std::cout << "✗ Socket 2 did not receive broadcast" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Failed to send message from Socket 1" << std::endl;
        }
    } 
    else 
    {
        std::cout << "⚠ Skipping broadcast test (missing socket)" << std::endl;
    }
    std::cout << "===================================================\n" << std::endl;

    // ---- Test 5: Socket disconnection cleanup ----
    std::cout << "============================================" << std::endl;
    std::cout << "5) Testing socket disconnect cleanup" << std::endl;
    if (clientSock1 >= 0) 
    {
        close(clientSock1);
        clientSock1 = -1;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int countAfter = server.get_connection_count();
        if (countAfter == 1)
            std::cout << "✓ Server cleaned up Socket 1" << std::endl;
        else
            std::cout << "✗ Unexpected socket count after disconnect: " << countAfter << std::endl;
    }
    else 
    {
        std::cout << "⚠ Skipping disconnect test (Socket 1 missing)" << std::endl;
    }
    std::cout << "============================================\n" << std::endl;

    // ---- Test 6: Cleanup on disconnect ----
    std::cout << "==========================================================" << std::endl;
    std::cout << "6) Testing cleanup of last client socket" << std::endl;
    if (clientSock2 >= 0) 
    {
        close(clientSock2);
        clientSock2 = -1;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (server.get_connection_count() == 0)
            std::cout << "✓ Server cleaned up Socket 2" << std::endl;
        else
            std::cout << "✗ Server still reports " << server.get_connection_count() << " sockets" << std::endl;
    }
    else 
    {
        std::cout << "⚠ Skipping cleanup test (Socket 2 missing)" << std::endl;
    }
    std::cout << "==========================================================\n" << std::endl;

    // ---- Test 7: Server shutdown ----
    std::cout << "==================================" << std::endl;
    std::cout << "7) Testing server shutdown" << std::endl;
    server.stop();
    if (serverThread.joinable()) serverThread.join();
    std::cout << "✓ Server stopped and thread joined" << std::endl;
    std::cout << "==================================\n" << std::endl;

    std::cout << "✓✓✓ All tests finished" << std::endl;
    return 0;
}
