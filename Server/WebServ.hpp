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
#include <signal.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 80000
#define TIME_OUT 5000
#define INACTIVITY_TIMEOUT 30

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

// class Client;
struct ClientData
{
	std::string chunk;
	std::string boundary;
	std::string request;
	int tries;
	size_t rcl;
	size_t header_length;
	size_t wcl;
	ssize_t bytes_received;
	ssize_t sent_bytes;
	time_t last_activity_time;

	bool headerDataSet;
	bool removeHeader;
	bool clientServed;

	ClientData()
	{
		this->rcl = 0;
		this->header_length = 0;
		this->wcl = 0;
		this->bytes_received = -1;
		this->headerDataSet = false;
		this->removeHeader = false;
		this->clientServed = false;
		this->sent_bytes = 0;
		this->tries = 0;
	}
};

class WebServ
{
private:
	sockaddr_in hint;
	std::map<int, ClientData> clientDataMap;
	std::vector<pollfd> fds;
	std::vector<int> listeners;
	std::vector<int> ports;
	Config *config;

public:
	WebServ(Config *config);
	~WebServ();
	int init(std::string host, const int port);
	void initServers();
	int setNonBlockingMode(int socket);
	void handleServersIncomingConnections();
	void socketConfig(std::string host, const int port);
	void AddSocket(int socket, bool isListener, int event);
	int acceptConnectionsFromListner(int listener);

	void cleanUp(int client_socket, size_t &i);
	int getClientIndex(int client_socket);
	void cleanUpInactiveClients();

	void handleClientsRequest(int client_socket, size_t &i);
	int getHeaderData(int client_socket, bool *flag, std::string &boundary, ssize_t bytes_received, char *buffer);
	void handlePostRequest(int client_socket, char *buffer, ssize_t bytes_received, std::string &boundary);
	void parseFormDataChunked(int client_socket, std::string &boundary, std::string &chunk);
	void setClientWritable(int client_socket);
	void parseFormDataContentLength(int client_socket, std::string &boundary, std::string &chunk, size_t &rcl, size_t &wcl);

	std::vector<int> getListeners() const;
	std::map<int, FileUpload> BodyMap;
	std::map<int, Client> clients;
};