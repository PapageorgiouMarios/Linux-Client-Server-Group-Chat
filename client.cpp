#include "client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <errno.h>

Client::Client(const std::string& host, int port, const std::string& name)
    : host_(host), port_(port), name_(name), sockfd_(-1), running_(false) {} // Constructor

Client::~Client() 
{
    disconnect(); // Ensure disconnection on destruction
}

bool Client::connectToServer() 
{
    if (sockfd_ != -1) return true; // Already connected

    struct addrinfo hints; // Hints for getaddrinfo
    struct addrinfo* res = nullptr; // Resulting address info
    memset(&hints, 0, sizeof(hints)); // All hint's fields are set to their default values
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    std::string portStr = std::to_string(port_); // Convert port to string
    int rc = getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &res); // Get address info
    
    if (rc != 0)
    {
        std::cerr << "✗ Can't get address info: " << gai_strerror(rc) << std::endl;
        return false;
    }

    struct addrinfo* p = res; // Pointer for iterating through results

    // Loop through all results
    for (; p != nullptr; p = p->ai_next)
    {
        sockfd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // Create socket
        if (sockfd_ == -1) continue; // Socket creation failed, try next

        if (connect(sockfd_, p->ai_addr, p->ai_addrlen) == -1)  // Attempt to connect
        { 
            close(sockfd_); // Close socket on failure
            sockfd_ = -1; // Reset sockfd
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (sockfd_ == -1) // No valid connection was made 
    {
        std::cerr << "✗ Unable to connect to " << host_ << ":" << port_ << std::endl;
        return false;
    }

    running_ = true;
    if (recv_thread_.joinable()) { recv_thread_.join(); } // Join any existing thread
    recv_thread_ = std::thread(&Client::receiveLoop, this); // Start the receive thread

    return true;
}

void Client::disconnect() 
{
    running_ = false;  // Stop the receive loop

    if (sockfd_ != -1) 
    {
        shutdown(sockfd_, SHUT_RDWR); // Disable further send/receive operations
        close(sockfd_); // Close the socket
        sockfd_ = -1; // Reset sockfd
    }

    std::thread localThread; // Local thread to join outside of lock
    
    if (recv_thread_.joinable()) 
    {
        localThread = std::move(recv_thread_); // Move to local thread to avoid joining self
    }

    if (localThread.joinable()) 
    {
        if (localThread.get_id() == std::this_thread::get_id())  // Check if trying to join self
        {
            std::cerr << "⚠ Disconnect attempted to join current thread; detaching instead\n";
            localThread.detach();
        } 
        else 
        {
            try 
            {
                localThread.join();
            } 
            catch (const std::system_error& e) 
            {
                std::cerr << "✗ Join failed: " << e.what() << std::endl;
            }
        }
    }

    if (recv_thread_.joinable()) recv_thread_.join();
}

bool Client::isConnected() const { return running_ && sockfd_ != -1;}

bool Client::sendAll(const char* data, size_t len) 
{
    size_t totalSent = 0; // Total bytes sent so far
    while (totalSent < len) 
    {
        ssize_t sent = send(sockfd_, data + totalSent, len - totalSent, 0); // Send remaining data
        if (sent <= 0) 
        {
            return false; // error or connection closed
        }
        totalSent += static_cast<size_t>(sent);
    }
    return true;
}

bool Client::sendMessage(const std::string& message) // Send a string message to the server
{
    if (!isConnected()) return false;
    std::string out = message;
    if (!name_.empty()) { out = name_ + ": " + message; } // Add client's name
    return sendAll(out.c_str(), out.size()); // Send raw bytes. Server reads and broadcasts them.
}

void Client::receiveLoop()
{
    const size_t BUF_SIZE = 4096; // Buffer size for receiving data
    char buffer[BUF_SIZE]; // Receive buffer

    while (running_)  
    {
        memset(buffer, 0, BUF_SIZE); // Clear the buffer
        ssize_t recvd = recv(sockfd_, buffer, BUF_SIZE, 0); // Receive data
        if (recvd > 0) // Data received
        {
            // Print received message to stdout
            std::string msg(buffer, static_cast<size_t>(recvd)); // Convert to string
            std::cout << msg << std::endl;
        } 
        else if (recvd == 0) 
        {
            std::cout << "⚠ Server closed connection" << std::endl;
            running_ = false;
            break;
        } 
        else 
        {
            if (errno == EINTR) continue; // Interrupted, retry
            std::cerr << "✗ Receive function: " << strerror(errno) << std::endl;
            running_ = false;
            break;
        }
    }
}
