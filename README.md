# 🖥️ Linux Multithreaded Client-Server Group Chat
[![C++](https://img.shields.io/badge/C%2B%2B-00599C?logo=cplusplus&logoColor=white&style=flat-square)](https://isocpp.org) [![Docker](https://img.shields.io/badge/Docker-257BD6?logo=docker&logoColor=white&style=flat-square)](https://www.docker.com) [![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black&style=flat-square)](https://www.kernel.org)

This project is a multithreaded TCP-based chat system in C++ for Linux. It allows multiple clients to connect to a central server, exchange text messages in real time, and supports containerized deployment with Docker.

---
### 🧩 Core components 
- **Server**:  
  - Binds to an IP (0.0.0.0) and port (default 8080)  
  - Listens for incoming TCP connections  
  - Maintains a registry of connected clients  
  - Broadcasts messages from any client to all others  
  - Enqueues messages (with metadata) into a message queue for logging or further processing
  
- **Client**:  
  - Connects to the server  
  - Reads user input and sends it to the server  
  - Listens for broadcasts from the server and displays them

  ---
 ### 🧵 Threading Model

- The server runs an **acceptor thread**, which continuously waits for new connections (`accept()`).  
- For every new client connection, the server launches a **dedicated client handler thread**.  
  - Each handler thread loops on `recv()` to receive messages from that client.  
  - Upon receiving a message, it calls the broadcast logic to forward it to all other clients.  
  - If the client disconnects (or errors), the handler cleans up and exits.  
- Shared resources (e.g. list of client sockets, message queue) are protected with mutexes and condition variables to ensure thread safety.

 ---
### 📨 Message flow

1. A client sends a text message via `send()`.  
2. The server’s handler thread reads it using `recv()`.  
3. The handler wraps the bytes into a `std::string` and calls `broadcast()`.  
4. In `broadcast()`:
   - Lock `clients_mutex`  
   - Iterate over all client sockets (except the sender)  
   - `send()` the message to each  
   - Unlock `clients_mutex`  
   - Also, call `addMessageToQueue()` to enqueue the message  
5. The queue consumer (if present) wakes via `queueCv`, locks, dequeues, and processes the message (e.g. log, archive).  

---
### 🐋 Run project using containers
Because development happened on Windows, Docker is used to easily run and test the project across environments.

1) Build the Container Images
```bash
   docker-compose -f docker/docker-compose.yml build --no-cache
```
2) Start the containers
```bash
  docker-compose -f docker/docker-compose.yml up --no-build
```
You should see output similar to:
```bash
  [+] Running 4/4
   ✔ Container docker-server-test-1  Created                                                                                                                                                           
   ✔ Container docker-client-test-1  Created                                                                                                                                                            
   ✔ Container docker-main-server-1  Created                                                                                                                                                            
   ✔ Container docker-main-client-1  Created  
```
The test containers will run sample tests and print results. Meanwhile, the main server container stays running, waiting for real clients.

3) Launch clients
```bash
  docker ps
  docker exec -it <container id> /bin/bash
  cd build
  ./main_client main-server 8080
```
- Use multiple terminals to create multiple clients connected to the same server
  
4) Chat away! 💬





