#include <string>
#include <thread>
#include <atomic>

class Client {
public:
    
    Client(const std::string& host, int port, const std::string& name = ""); // Constructor
    ~Client(); // Destructor

    bool connectToServer(); // Connect to the server. Returns true on success.
    void disconnect(); // Disconnect and cleanup from server
    bool sendMessage(const std::string& message); // Send a single message (strings only)
    bool isConnected() const; // Check if the client is connected to the server

private:
    void receiveLoop(); // Thread function to receive messages while running
    bool sendAll(const char* data, size_t len); // Ensure message's data are sent

    std::string host_; // Server hostname or IP
    int port_; // Server port
    std::string name_; // Client's name
    int sockfd_; // Socket file descriptor

    std::thread recv_thread_; // Thread for receiving messages
    std::atomic<bool> running_; // Flag to control the receive thread
};
