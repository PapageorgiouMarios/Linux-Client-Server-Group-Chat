#include "client.h"
#include "server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <unistd.h>

int main() 
{
    std::cout << "=== Client Test Suite ===\n";

    const int port = 9999;
    const std::string host = "0.0.0.0";

    // Start server
    Server server(port);
    
    std::thread serverThread([&server]() { server.start(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 1) Single client connects
    std::cout << "===========================================" << std::endl;
    std::cout << "1) Testing single client connection" << std::endl;
    {
        Client c1(host, port, "Alice");

        if (c1.connectToServer()) 
        {
            std::cout << "✓ Alice connected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Alice failed to connect" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        int connCount = server.get_connection_count();

        if (connCount >= 1) 
        {
            std::cout << "✓ Server reports " << connCount << " client(s) connected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Server did not register client(s)" << std::endl;
        }

        c1.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        connCount = server.get_connection_count();

        if (connCount == 0) 
        {
            std::cout << "✓ Alice disconnected successfully" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Unexpected connection count after disconnect: " << connCount << std::endl;
        }
    }
    std::cout << "===========================================\n" << std::endl;

    // 2) Multiple clients connect
    std::cout << "==============================================" << std::endl;
    std::cout << "2) Testing multiple clients connection" << std::endl;
    {
        Client c1(host, port, "Bob");
        Client c2(host, port, "Carol");

        bool ok1 = c1.connectToServer();
        bool ok2 = c2.connectToServer();

        if (ok1) std::cout << "✓ Bob connected" << std::endl; else std::cout << "✗ Bob failed to connect" << std::endl;
        if (ok2) std::cout << "✓ Carol connected" << std::endl; else std::cout << "✗ Carol failed to connect" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        int connCount = server.get_connection_count();

        if (connCount > 0 && connCount <= 2) 
        {
            std::cout << "✓ Server reports " << connCount << " client(s) connected as expected" << std::endl;
        }
        else if (connCount > 2)
        {
            std::cout << "✗ Server reports " << connCount << " more clients than expected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Server did not register multiple clients" << std::endl;
        }

        c1.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        c2.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        connCount = server.get_connection_count();
        
        if (connCount == 0) 
        {
            std::cout << "✓ Bob and Carol disconnected successfully" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Unexpected connection count after cleanup: " << connCount << std::endl;
        }
    }
    std::cout << "==============================================\n" << std::endl;

    // 3) Disconnect and reconnect
    std::cout << "=========================================================" << std::endl;
    std::cout << "3) Testing client disconnection and re-connection" << std::endl;
    {
        Client c1(host, port, "Dave");

        if (c1.connectToServer()) 
        {
            std::cout << "✓ Dave connected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Dave failed to connect" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        c1.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        int connCount = server.get_connection_count();

        if (connCount == 0) 
        {
            std::cout << "✓ Dave disconnected successfully" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Unexpected connection count after disconnect: " << connCount << std::endl;
        }

        // Reconnection
        if (c1.connectToServer()) 
        {
            std::cout << "✓ Dave reconnected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Dave failed to reconnect" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        connCount = server.get_connection_count();
        if (connCount >= 1) 
        {
            std::cout << "✓ Server registered reconnection" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Server did not register reconnection" << std::endl;
        }

        c1.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "=========================================================\n" << std::endl;

    // 4) Message broadcasting and receiving
    std::cout << "=======================================" << std::endl;
    std::cout << "4) Testing message broadcasting" << std::endl;
    {
        Client sender(host, port, "Eve");
        Client receiver(host, port, "Frank");

        bool okS = sender.connectToServer();
        bool okR = receiver.connectToServer();

        if (okS) std::cout << "✓ Eve connected" << std::endl; else std::cout << "✗ Eve failed to connect" << std::endl;
        if (okR) std::cout << "✓ Frank connected" << std::endl; else std::cout << "✗ Frank failed to connect" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::string testMsg = "Hello from Eve";

        if (sender.sendMessage(testMsg)) 
        {
            std::cout << "✓ Eve sent message: " << testMsg << std::endl;
        } 
        else 
        {
            std::cout << "✗ Eve failed to send message" << std::endl;
        }

        // Give time for broadcast to propagate
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        sender.disconnect();
        receiver.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "=======================================\n" << std::endl;

    // 5) Print server's message queue (explicit test)
    std::cout << "========================================" << std::endl;
    std::cout << "5) Testing message queue display" << std::endl;
    {
        std::cout << "Print all messsages sent" << std::endl;
        server.printMessageQueue();
    }
    std::cout << "========================================\n" << std::endl;

    // 6) Disconnect after server shutdown
    std::cout << "====================================================" << std::endl;
    std::cout << "6) Testing server shutdown and disconnection" << std::endl;
    {
        Client c1(host, port, "Grace");

        if (c1.connectToServer()) 
        {
            std::cout << "✓ Grace connected" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Grace failed to connect" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Stop server
        server.stop();
        if (serverThread.joinable()) serverThread.join();

        auto waitTimeoutMs = 2000;
        auto stepMs = 50;
        bool detected = false;

        for (int waited = 0; waited < waitTimeoutMs; waited += stepMs) 
        {
            if (!c1.isConnected()) 
            {
                detected = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(stepMs));
        }

        if (detected) 
        {
            std::cout << "✓ Grace detected server shutdown" << std::endl;
        } 
        else 
        {
            std::cout << "✗ Grace still reports connected after server shutdown" << std::endl;
            c1.disconnect();
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            if (!c1.isConnected()) 
            {
                std::cout << "✗ Grace disconnected after explicit disconnect()" << std::endl;
            } 
            else 
            {
                std::cout << "✗ Grace did not disconnect even after explicit disconnect()" << std::endl;
            }
        }
        c1.disconnect();
    }
    std::cout << "====================================================\n" << std::endl;
    
    std::cout << "✓✓✓ All tests finished" << std::endl;
    return 0;
}
