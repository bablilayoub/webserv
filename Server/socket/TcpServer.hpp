#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>

#define PORT 8080
#define MAX_CLIENTS 20
#define MAX_BYTES_TO_SEND 10000

class TcpServer
{
private:
	struct sockaddr_in serverAddress;
	bool isNonBlocking;
	int listener;

public:
	TcpServer();
	void initializeServer(const int port);
	int handleIncomingConnections();
	void setNonBlockingMode(int socket);
	void socketConfig(const int port);
	void closeFds(std::vector<pollfd> &poll_fds_vec);
	void AddClientSocket(std::vector<pollfd> &poll_fds_vec, int client_socket);
	int accept_IncomingConnection(std::vector<pollfd> &poll_fds_vec, size_t i);
	void handle_clietns(std::vector<pollfd> &poll_fds_vec, size_t i);
};