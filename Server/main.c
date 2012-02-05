//
//  main.c
//  Server
//
//  Created by Matthew Dahl on 1/31/12.
//  Copyright 2012 Mintrus, LLC. All rights reserved.
//

#define PORT 80

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *message);

int main(int argc, char *argv[])
{
    char buffer[256];
    pid_t pid;
    struct sockaddr_in client;
    socklen_t clientLength;
    int newSocketHandle;
    int numChars;
    int optval;
    struct sockaddr_in server;
    int socketHandle;
    
    // Create a TCP socket
    socketHandle = socket(AF_INET, SOCK_STREAM, 0);
    if (socketHandle < 0) error("ERROR opening socket");
    
    // Set socket option
    optval = 1;
    setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    // Configure server
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    
    // Bind socket to port
    if (bind(socketHandle, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        close(socketHandle);
        error("ERROR on binding");
    }
    
    // Listen to socket on port
    if (listen(socketHandle, SOMAXCONN) != 0)
    {
        close(socketHandle);
        error("ERROR listening to port");
    }
    
    for(;;)
    {
        // Accept new connection
        clientLength = sizeof(client);
        newSocketHandle = accept(socketHandle, (struct sockaddr *) &client, &clientLength);
        if (newSocketHandle < 0) error("ERROR on accept");
        
        pid = fork();
        
        if (pid < 0) // Error forking
        {
            close(newSocketHandle);
            error("ERROR while forking");
        }
        else if (pid == 0) // Child process
        {
            // Get name of host
            getpeername(newSocketHandle, (struct sockaddr *) &client, &clientLength);
            char * clientIP = inet_ntoa(client.sin_addr);
            printf("Connection to %s\n", clientIP);
            
            // Read data
            size_t bufferLength = sizeof(buffer);
            bzero(buffer, bufferLength);
            numChars = read(newSocketHandle, buffer, bufferLength);
            if (numChars < 0) error("ERROR reading from socket");
            
            // Write read data to console
            printf("Here is the message: %s\n",buffer);
            
            // Write data to connection
            numChars = write(newSocketHandle, "I got your message", 18);
            if (numChars < 0) error("ERROR writing to socket");
            
            // Close file handle
            close(newSocketHandle);
        }
        else // Parent process
        {
            // Close file handle
            close(newSocketHandle);
        }
    }
    
    // Close file handle
    close(socketHandle);
    
    return 0; 
}

void error(const char *message)
{
    perror(message);
    exit(-1);
}
