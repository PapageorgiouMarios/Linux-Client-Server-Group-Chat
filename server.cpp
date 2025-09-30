#include "server.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <algorithm>

Server::Server(int port) : port(port), running(false), listening(-1) {} // Constructor

Server::~Server() 
{
    stop(); // Ensure server is stopped on destruction
}

void Server::start() 
{
    listening = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    
    if (listening == -1) // Check for socket creation error
    {
        std::cerr << "✗ Can't create server socket!" << std::endl;
        return;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // Use default IP address 0.0.0.0

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) // Bind the socket to the IP/port
    {
        std::cerr << "✗ Can't bind to port!" << std::endl;
        return;
    }

    if (listen(listening, SOMAXCONN) == -1) // Mark the socket for listening
    {
        std::cerr << "✗ Can't listen!" << std::endl;
        return;
    }

    running = true;
    std::thread(&Server::acceptClients, this).detach(); // Start accepting clients in a separate thread
    std::cout << "🖥 Server started on port " << port << std::endl;
}

void Server::stop() 
{
    running = false;
    if (listening != -1) 
    {
        shutdown(listening, SHUT_RDWR); // Disable further send/receive operations
        close(listening); // Close the listening socket
        listening = -1;
    }

    std::lock_guard<std::mutex> lock(clients_mutex); // Lock the clients list for safe access
    
    for (int clientSock : client_sockets)  // Close all client sockets
    {
        shutdown(clientSock, SHUT_RDWR);
        close(clientSock);
    }

    client_sockets.clear(); // Clear the client sockets list

    for (std::thread& t : client_threads)  // Join all client handling threads
    {
        if (t.joinable()) // Check if thread is joinable
        {
            t.join();
        }
    }
}

int Server::get_connection_count() 
{
    std::lock_guard<std::mutex> lock(clients_mutex); // Lock the clients list for safe access
    return client_sockets.size(); // Return the number of active clients
}

void Server::remove_client(int clientSock)  // Remove a client from the list
{
    std::lock_guard<std::mutex> lock(clients_mutex); // Lock the clients list for safe access
    auto it = std::find(client_sockets.begin(), client_sockets.end(), clientSock); // Find the client socket

    if (it != client_sockets.end())  
    {
        close(clientSock);
        client_sockets.erase(it);
    }
}

void Server::handleClient(int clientSock) // Handle communication with a client
{
    char buf[4096]; // Buffer for receiving data

    while (running) 
    {
        memset(buf, 0, sizeof(buf));
        int bytesReceived = recv(clientSock, buf, sizeof(buf), 0); // Receive data from client
        
        if (bytesReceived <= 0) 
        {
            std::cout << "⚠ Client disconnected" << std::endl;
            remove_client(clientSock);
            break;
        }

        std::string msg(buf, bytesReceived);
        std::cout << "✉  " << msg << std::endl;
        broadcast(msg, clientSock); // Broadcast message to other clients
    }
}

void Server::broadcast(const std::string& message, int senderSock) 
{
    std::lock_guard<std::mutex> lock(clients_mutex); // Lock the clients list for safe access
    
    for (int clientSock : client_sockets) // Send message to all clients except the sender 
    {
        if (clientSock != senderSock) 
        {
            send(clientSock, message.c_str(), message.size(), 0); // Send the message
        }
    }

    // std::cout << "Broadcasted message to clients" << std::endl;
    addMessageToQueue(message, senderSock); // Add message to the queue for broadcasting
}

void Server::acceptClients() {
    while (running) {
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        // Attempt to accept a new client connection
        int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

        // Check if a new connection was accepted
        if (clientSocket != -1) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                client_sockets.push_back(clientSocket);
            }

            // Start a new thread to handle the client's communication
            std::thread(&Server::handleClient, this, clientSocket).detach();

            // Log the new connection
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << "✓ New client connected from " << clientIP << std::endl;
        } else {
            // No new connection, perform any necessary housekeeping
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Server::addMessageToQueue(const std::string& message, int senderSock) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push({message, senderSock}); // Add message and sender socket to the queue
    }
    queueCv.notify_one(); // Notify the broadcasting thread
}

void Server::printMessageQueue() {
    std::cout << "Message Queue Size: " << messageQueue.size() << std::endl;
    std::queue<std::pair<std::string, int>> tempQueue = messageQueue; // Copy to a temporary queue for printing

    while (!tempQueue.empty()) 
    {
        auto& msgPair = tempQueue.front(); // Get the front message
        std::cout <<  msgPair.first << std::endl;
        std::cout << "Sender Socket: " << msgPair.second << std::endl;
        tempQueue.pop(); // Remove the front message
    }
}