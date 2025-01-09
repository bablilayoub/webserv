#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define PORT 8080

class TcpServer {
private:
	struct sockaddr_in serverAddress;
	int sockfd;
public:
	TcpServer();
	int socketCreationAndBinding();
	int handleIncomingConnection();
};