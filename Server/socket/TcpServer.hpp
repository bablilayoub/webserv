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
#include "../../Client/Config.hpp"
#include "../../FileUpload/FileUpload.hpp"
#include <map>

#define PORT 9000
#define MAX_BYTES_TO_SEND 200000
#define BUFFER_SIZE 60001
#define TIME_OUT 5000

struct ClientData
{
	std::string chunk;
	size_t received_content_length;
	size_t header_length;
	size_t wholeContentLength;
	ssize_t bytes_received;
	int flag;
	int length_set;
	std::string file_name;

	ClientData()
	{
		this->received_content_length = 0;
		this->header_length = 0;
		this->wholeContentLength = 0;
		this->bytes_received = -1;
		this->flag = true;
		this->length_set = false;
	}
};

class TcpServer
{
private:
	struct sockaddr_in serverAddress;
	std::vector<ClientData> clientData;
	std::vector<pollfd> poll_fds_vec;
	int listener;
	bool isNonBlocking;
	Config* config;

public:
	TcpServer(Config* config);
	void initializeServer(const int port);
	int handleIncomingConnections();
	void setNonBlockingMode(int socket);
	void socketConfig(const int port);
	// void closeFds(std::vector<pollfd> &poll_fds_vec);
	void AddClientSocket(int client_socket);
	int accept_IncomingConnection();
	void handle_clients(size_t *i);
	size_t findContentLength(int client_socket, int *flag);
	std::map<int, FileUpload> BodyMap;
	std::map<int, Client> clients;
};