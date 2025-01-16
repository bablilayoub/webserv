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
#include "../Client/Client.hpp"
#include "../FileUpload/FileUpload.hpp"
#include "../Client/Config.hpp"
#include <map>

#define PORT 9000
#define BUFFER_SIZE 80001
#define TIME_OUT 5000
#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

struct ClientData
{
	std::string chunk;
	std::string boundary;
	size_t received_content_length;
	size_t header_length;
	size_t wholeContentLength;
	ssize_t bytes_received;
	bool headerDataSet;
	bool removeHeader;

	ClientData()
	{
		this->received_content_length = 0;
		this->header_length = 0;
		this->wholeContentLength = 0;
		this->bytes_received = -1;
		this->headerDataSet = false;
		this->removeHeader = false;
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
	Config *config;

public:
	TcpServer(Config *config);
	void initializeServer(const int port);
	int handleIncomingConnections();
	void setNonBlockingMode(int socket);
	void socketConfig(const int port);
	void closeFds();
	void AddClientSocket(int socket, int event);
	int acceptIncomingConnection();
	void handleClientsRequest(int client_socket, size_t *i);
	void getHeaderData(int client_socket, bool *flag, size_t *i, std::string &boundary);
	void handlePostRequest(int client_socket, char *buffer, ssize_t bytes_received, size_t *i, std::string &boundary);
	void cleanUp(int client_socket, size_t *i);
	void parseIfContentLength(int client_socket, std::string &boundary, std::string &chunk, size_t *i, size_t &received_content_length, size_t &wholeContentLength);
	void fileReachedEnd(std::string &chunk, int client_socket, size_t &received_content_length, size_t &wholeContentLength, size_t *i);
	std::map<int, FileUpload> BodyMap;
	std::map<int, Client> clients;
};