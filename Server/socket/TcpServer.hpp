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
#define MAX_CLIENTS 5
#define MAX_BYTES_TO_SEND 200000
#define BUFFER_SIZE 60001
#define TIME_OUT 5000

class IncomingDataHandler
{
public:
	std::string chunk;
	size_t received_content_length;
	size_t header_length;
	size_t wholeContentLength;
	ssize_t bytes_received;

	IncomingDataHandler()
	{
		this->received_content_length = 0;
		this->header_length = 0;
		this->wholeContentLength = 0;
		this->bytes_received = -1;
	}
};

class TcpServer
{
private:
	struct sockaddr_in serverAddress;
	int listener;
	bool isNonBlocking;
	size_t received_content_length;
	size_t header_length;
	std::vector<IncomingDataHandler> incomingDataHandlers;

public:
	TcpServer();
	void initializeServer(const int port);
	int handleIncomingConnections();
	void setNonBlockingMode(int socket);
	void socketConfig(const int port);
	void closeFds(std::vector<pollfd> &poll_fds_vec);
	void AddClientSocket(std::vector<pollfd> &poll_fds_vec, int client_socket);
	int accept_IncomingConnection(std::vector<pollfd> &poll_fds_vec, size_t i);
	void handle_clients(std::vector<pollfd> &poll_fds_vec, size_t *i);
	size_t findContentLength(int client_socket);
};