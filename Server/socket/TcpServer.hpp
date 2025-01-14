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
#include "../../Client/Client.hpp"
#include "../../FileUpload/FileUpload.hpp"
#include <map>

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_BYTES_TO_SEND 200000
#define BUFFER_SIZE 60001
#define TIME_OUT 5000

class TcpServer
{
private:
	struct sockaddr_in serverAddress;
	int listener;
	bool isNonBlocking;
	size_t received_content_length;
	size_t header_length;	

public:
	TcpServer();
	void initializeServer(const int port);
	int handleIncomingConnections();
	void setNonBlockingMode(int socket);
	void socketConfig(const int port);
	void closeFds(std::vector<pollfd> &poll_fds_vec);
	void AddClientSocket(std::vector<pollfd> &poll_fds_vec, int client_socket);
	int accept_IncomingConnection(std::vector<pollfd> &poll_fds_vec, Client &client);
	void handle_clients(std::vector<pollfd> &poll_fds_vec, size_t *i, Client &client);
	size_t findContentLength(int client_socket);
	std::map<int, FileUpload> BodyMap;
};