#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "CricketAnalyzer.h"

class NetworkServer {
private:
    int serverSocket;
    vector<int> clientSockets;
    mutex clientMutex;
    bool isRunning;
    CricketAnalyzer* analyzer;
    
public:
    NetworkServer(CricketAnalyzer* analyzerObj) {
        serverSocket = -1;
        isRunning = false;
        analyzer = analyzerObj;
        
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    
    ~NetworkServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    bool start(int port = 8080) {
        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            cout << "Error: Cannot create socket" << endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        // Bind socket
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            cout << "Error: Cannot bind socket" << endl;
            return false;
        }
        
        // Listen for connections
        if (listen(serverSocket, 5) < 0) {
            cout << "Error: Cannot listen on socket" << endl;
            return false;
        } 
        isRunning = true;
        cout << "Server started on port " << port << endl;
        
        // Start accepting clients in separate thread
        thread acceptThread(&NetworkServer::acceptClients, this);
        acceptThread.detach();
        
        return true;
    }
    
    void stop() {
        isRunning = false;
        
        // Close all client sockets
        lock_guard<mutex> lock(clientMutex);
        for (int sock : clientSockets) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
        }
        clientSockets.clear();
        
        // Close server socket
        if (serverSocket >= 0) {
#ifdef _WIN32
            closesocket(serverSocket);
#else
            close(serverSocket);
#endif
            serverSocket = -1;
        }
    }
    
    void broadcastEvent(const CricketEvent& event) {
        // Convert event to JSON
        stringstream ss;
        ss << "{" << endl;
        ss << "  \"type\": " << event.type << "," << endl;
        ss << "  \"timestamp\": " << event.timestamp << "," << endl;
        ss << "  \"confidence\": " << event.confidence << "," << endl;
        ss << "  \"description\": \"" << event.description << "\"" << endl;
        ss << "}" << endl;
        
        string jsonData = ss.str();
        // Send to all clients
        lock_guard<mutex> lock(clientMutex);
        for (int sock : clientSockets) {
            send(sock, jsonData.c_str(), jsonData.length(), 0);
        }
    }
    
private:
    void acceptClients() {
        while (isRunning) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket >= 0) {
                lock_guard<mutex> lock(clientMutex);
                clientSockets.push_back(clientSocket);
                cout << "New client connected. Total clients: " 
                     << clientSockets.size() << endl;
                
                // Send welcome message
                string welcome = "Connected to Cricket Highlights Server\n";
                send(clientSocket, welcome.c_str(), welcome.length(), 0);
            }
            
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
};

#endif