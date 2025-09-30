#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

class Server {
public:
    Server(int port); // Constructor
    ~Server(); // Destructor

    void start(); // Start the server | Open to connections
    void stop(); // Stop the server | Close all connections  

    int get_connection_count(); /// Find number of active clients

    void remove_client(int clientSock); // Remove a client from the list   
    void handleClient(int clientSock); // Handle communication with a client
    void broadcast(const std::string& message, int senderSock); // Display one client's message to other clients 
    void acceptClients(); // Accept incoming client connections

    void addMessageToQueue(const std::string& message, int senderSock); // Add message to the queue for broadcasting
    void printMessageQueue(); // Print the message queue (for debugging)
    bool running; // Server running status

private:
    int port; // Port number            
    int listening; // Listening socket
    
    std::queue<std::pair<std::string, int>> messageQueue; // Queue for messages to broadcast
    std::mutex queueMutex; // Mutex for thread-safe queue access
    std::condition_variable queueCv; // Condition variable for message notification

    std::vector<int> client_sockets; // List of active client sockets
    std::vector<std::thread> client_threads; // Threads representing each client connection
    std::mutex clients_mutex; // Mutex for thread-safe access to client_sockets   
};
