#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

class TcpServer
{
private:
	struct sockaddr_in serverAddress;
	bool isNonBlocking;
	int sockfd;

public:
	TcpServer();
	void initializeServer(const int PORT);
	int handleIncomingConnections();
	void setNonBlockingMode();
	void socketConfig(const int PORT);
};