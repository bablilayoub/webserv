#include "WebServ.hpp"

////////////////////////////////////////
///           SERVER SETUP           ///
////////////////////////////////////////

WebServ::WebServ(Config *config) : config(config) {}

WebServ::~WebServ()
{
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (fds[i].fd > 0)
			close(fds[i].fd);
	}
}

int WebServ::init(std::string host, const int port)
{
	int listener = -1;
	int retryCount = 0;
	const int maxRetries = 10;
	while (retryCount < maxRetries)
	{
		listener = socket(AF_INET, SOCK_STREAM, 0);
		if (listener == INVALID_SOCKET)
		{
			std::cout << "Socket creation failed" << std::endl;
			retryCount++;
			continue;
		}

		int optval = 1;
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == SOCKET_ERROR)
		{
			close(listener);
			std::cout << "setsockopt failed" << std::endl;
			retryCount++;
			continue;
		}

		if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == SOCKET_ERROR)
		{
			close(listener);
			std::cout << "setsockopt failed" << std::endl;
			retryCount++;
			continue;
		}

		this->socketConfig(host, port);
		if (this->setNonBlockingMode(listener) == -1)
		{
			close(listener);
			std::cout << "Failed to set non-blocking mode" << std::endl;
			retryCount++;
			continue;
		}

		if (bind(listener, (sockaddr *)&this->hint, sizeof(this->hint)) == SOCKET_ERROR)
		{
			close(listener);
			std::cout << "bind failed" << std::endl;
			retryCount++;
			continue;
		}

		if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
		{
			close(listener);
			std::cout << "Listening to server failed" << std::endl;
			retryCount++;
			continue;
		}
		return listener;
	}
	std::cerr << "Failed to initialize server on port " << port << " after " << maxRetries << " attempts." << std::endl;
	return -1;
}

void WebServ::socketConfig(std::string host, const int port)
{
	memset(&this->hint, 0, sizeof(this->hint));
	this->hint.sin_family = AF_INET;
	this->hint.sin_port = htons(port);
	inet_pton(AF_INET, host.c_str(), &this->hint.sin_addr);
};

int WebServ::setNonBlockingMode(int socket)
{
	if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0)
	{
		close(socket);
		return -1;
	}
	return 0;
}

void WebServ::initServers()
{
	size_t serversSize = this->config->servers.size();
	std::string host;
	std::vector<int> ports;
	int listener;
	std::vector<std::string> added_servers;

	for (size_t i = 0; i < serversSize; i++)
	{
		host = this->config->servers[i].host;
		ports = this->config->servers[i].ports;

		for (size_t j = 0; j < ports.size(); j++)
		{
			if (std::find(added_servers.begin(), added_servers.end(), std::string(host + ":" + std::to_string(ports[j]))) != added_servers.end())
				continue;

			added_servers.push_back(host + ":" + std::to_string(ports[j]));
			listener = this->init(host, ports[j]);
			if (listener == -1)
			{
				std::cout << "Server creation failed" << std::endl;
				continue;
			}
			this->listeners.push_back(listener);
			this->AddSocket(listener, true, POLLIN);
		}
	}
}

int WebServ::acceptConnectionsFromListner(int listener)
{
	int new_socket;
	sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	new_socket = accept(listener, (sockaddr *)&client_addr, &addr_len);
	if (new_socket == -1)
		return -1;
	this->AddSocket(new_socket, false, POLLIN);
	return 0;
}

void WebServ::AddSocket(int socket, bool isListener, int event)
{
	if (!isListener)
	{
		this->clients[socket] = Client();
		this->clients[socket].setup(socket, this->config);
		ClientData clientData;
		clientDataMap[socket] = clientData;
		clientDataMap[socket].last_activity_time = time(NULL);
		std::remove(("/tmp/cgi_input_" + std::to_string(socket)).c_str());
	}
	struct pollfd pfd;
	pfd.fd = socket;
	pfd.events = event;
	fds.push_back(pfd);
}

void WebServ::handleServersIncomingConnections()
{
	size_t maxTries = 0;
	while (true)
	{
		int ret = poll(&fds[0], fds.size(), TIME_OUT);
		if (ret == -1)
		{
			if (++maxTries > 10)
				throw std::runtime_error("Poll failed");
			continue;
		}
		maxTries = 0;
		cleanUpInactiveClients();
		if (ret == 0)
			continue;

		for (size_t i = 0; i < fds.size(); i++)
		{
			int client_socket = fds[i].fd;
			if (fds[i].revents & POLLIN)
			{
				if (std::find(listeners.begin(), listeners.end(), fds[i].fd) != listeners.end())
					acceptConnectionsFromListner(fds[i].fd);
				else
				{
					handleClientsRequest(fds[i].fd, i);
					clientDataMap[fds[i].fd].last_activity_time = time(NULL);
				}
			}
			if (fds[i].revents & POLLOUT)
			{
				if (!this->clients[client_socket].parsed)
					continue;

				this->clients[client_socket].generateResponse();

				this->clients[client_socket].sendResponse();

				if (this->clients[client_socket].response.done)
				{
					usleep(1000);
					cleanUp(client_socket, i);
				}
				else
					clientDataMap[fds[i].fd].last_activity_time = time(NULL);
			}
		}
	}
}

////////////////////////////////////////
///         CLEAR RESOURCES          ///
////////////////////////////////////////

void WebServ::cleanUp(int client_socket, size_t &i)
{
	this->clients.erase(client_socket);
	this->BodyMap.erase(client_socket);
	this->clientDataMap.erase(client_socket);
	fds.erase(fds.begin() + i);
	i--;
	close(client_socket);
}

void WebServ::cleanUpInactiveClients()
{
	time_t now = time(NULL);
	for (size_t i = 0; i < fds.size(); i++)
	{
		int fd = fds[i].fd;

		if (std::find(listeners.begin(), listeners.end(), fd) == listeners.end())
		{
			ClientData &data = clientDataMap[fd];
			if (now - data.last_activity_time > INACTIVITY_TIMEOUT)
				cleanUp(fd, i);
		}
	}
}

////////////////////////////////////////
///             PARSING              ///
////////////////////////////////////////

std::string getBoundary(std::string &header)
{
	size_t pos = header.find("boundary=");
	if (pos != std::string::npos)
	{
		size_t boundary_end = header.find("\r\n", pos);
		std::string boundary = header.substr(pos + 9, boundary_end - pos - 9);
		return "--" + boundary;
	}
	else
		return "";
}

int WebServ::getHeaderData(int client_socket, bool *flag, std::string &boundary, ssize_t bytes_received, char *buffer)
{
	std::string header;
	std::string &request = this->clientDataMap[client_socket].request;

	if (bytes_received > 0)
	{
		buffer[bytes_received] = '\0';
		request.append(buffer, bytes_received);
		size_t pos = request.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			header = request.substr(0, pos + 4);
			this->clients[client_socket].parse(header);
			this->clientDataMap[client_socket].wcl = this->clients[client_socket].getContentLength();
			boundary = getBoundary(header);
			this->clientDataMap[client_socket].chunk = request.substr(pos + 4);
			this->clientDataMap[client_socket].rcl = this->clientDataMap[client_socket].chunk.length();
			*flag = true;
			request.clear();
			return 0;
		}
	}
	return -1;
}

int WebServ::getClientIndex(int client_socket)
{
	for (size_t i = 0; i < fds.size(); ++i)
		if (fds[i].fd == client_socket)
			return i;
	return -1;
}

void WebServ::setClientWritable(int client_socket)
{
	size_t index = getClientIndex(client_socket);
	fds[index].events = POLLOUT;
}

void WebServ::parseFormDataChunked(int client_socket, std::string &boundary, std::string &chunk)
{
	bool flag = false;
	std::string boundaryString;

	size_t pos;
	if ((pos = chunk.find(boundary)) != std::string::npos)
	{
		if (pos != 0)
		{
			BodyMap[client_socket].ParseBody(chunk.substr(0, pos), boundary, this->clients[client_socket]);
			chunk = chunk.substr(pos);
			pos = 0;
		}
		size_t boundaryLength = boundary.length();
		boundaryString = chunk.substr(pos, boundaryLength);
		chunk = chunk.substr(pos + boundaryLength);
		size_t pos2;
		if ((pos2 = chunk.find(boundaryString)) != std::string::npos)
		{
			if (BodyMap[client_socket].ParseBody(boundaryString + chunk.substr(0, pos2), boundary, this->clients[client_socket]))
				setClientWritable(client_socket);
			chunk = chunk.substr(pos2);
			if (chunk.find(boundary + "--") != std::string::npos)
				if (BodyMap[client_socket].ParseBody(chunk, boundary, this->clients[client_socket]))
					setClientWritable(client_socket);
			return;
		}
		else
			flag = true;
	}
	if (BodyMap[client_socket].ParseBody((flag ? boundaryString : "") + chunk, boundary, this->clients[client_socket]))
		setClientWritable(client_socket);

	chunk.clear();
}

void WebServ::parseFormDataContentLength(int client_socket, std::string &boundary, std::string &chunk, size_t &rcl, size_t &wcl)
{
	bool flag = false;
	std::string boundaryString;

	size_t lastChunkPos;
	if ((lastChunkPos = chunk.find("\r\n" + boundary)) != std::string::npos)
		chunk = chunk.substr(0, lastChunkPos) + chunk.substr(lastChunkPos + 2);

	size_t pos;
	if ((pos = chunk.find(boundary)) != std::string::npos)
	{
		if (pos != 0)
		{
			BodyMap[client_socket].ParseBody(chunk.substr(0, pos), boundary, this->clients[client_socket]);
			chunk = chunk.substr(pos);
			pos = 0;
		}
		size_t boundaryLength = boundary.length();
		boundaryString = chunk.substr(pos, boundaryLength);
		chunk = chunk.substr(pos + boundaryLength);
		size_t pos2;
		if ((pos2 = chunk.find(boundaryString)) != std::string::npos)
		{
			BodyMap[client_socket].ParseBody(boundaryString + chunk.substr(0, pos2), boundary, this->clients[client_socket]);
			chunk = chunk.substr(pos2);
			if (rcl == wcl)
			{
				BodyMap[client_socket].ParseBody(chunk, this->clientDataMap[client_socket].boundary, this->clients[client_socket]);
				setClientWritable(client_socket);
			}
			return;
		}
		else
			flag = true;
	}
	BodyMap[client_socket].ParseBody((flag ? boundaryString : "") + chunk, boundary, this->clients[client_socket]);
	chunk.clear();
	if (rcl == wcl)
		setClientWritable(client_socket);
}

////////////////////////////////////////
///             CLIENTS              ///
////////////////////////////////////////

void WebServ::handlePostRequest(int client_socket, char *buffer, ssize_t bytes_received, std::string &boundary)
{
	size_t &wcl = this->clientDataMap[client_socket].wcl;
	size_t &rcl = this->clientDataMap[client_socket].rcl;
	std::string &chunk = this->clientDataMap[client_socket].chunk;
	std::ofstream cgiInput("/tmp/cgi_input_" + std::to_string(client_socket), std::ios::app);
	buffer[bytes_received] = '\0';
	chunk.append(buffer, bytes_received);
	rcl += bytes_received;

	if (this->clients[client_socket].getIsCGI())
	{
		cgiInput << chunk;
		chunk.clear();
		if (rcl == wcl)
		{
			cgiInput.close();
			setClientWritable(client_socket);
		}
	}
	else if (this->clients[client_socket].getContentType().find("multipart/form-data;") != std::string::npos)
	{
		if (this->clients[client_socket].getIsChunked())
			parseFormDataChunked(client_socket, boundary, chunk);
		else
			parseFormDataContentLength(client_socket, boundary, chunk, rcl, wcl);
	}
	else if (this->clients[client_socket].getIsBinary())
	{
		if (this->clients[client_socket].getIsChunked())
		{
			if (BodyMap[client_socket].ParseBody(chunk, "", this->clients[client_socket]))
				setClientWritable(client_socket);
			chunk.clear();
		}
		else
		{
			BodyMap[client_socket].ParseBody(chunk, "", this->clients[client_socket]);
			chunk.clear();
			if (rcl == wcl)
				setClientWritable(client_socket);
		}
	}
	else
		fds[getClientIndex(client_socket)].events = POLLOUT;
}

void WebServ::handleClientsRequest(int client_socket, size_t &i)
{
	ssize_t bytes_received = 0;
	char buffer[BUFFER_SIZE + 1];
	std::string &boundary = this->clientDataMap[client_socket].boundary;

	if (!this->clientDataMap[client_socket].headerDataSet || (this->clientDataMap[client_socket].headerDataSet && this->clients[client_socket].getMethod() == POST))
		bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
	if (bytes_received == 0)
	{
		fds[getClientIndex(client_socket)].events = POLLOUT;
		return;
	}

	if (!this->clientDataMap[client_socket].headerDataSet)
	{
		if (getHeaderData(client_socket, &this->clientDataMap[client_socket].headerDataSet, boundary, bytes_received, buffer) == -1) {
			return;
		}

		if (this->clients[client_socket].return_anyway || this->clients[client_socket].getMethod() != POST)
			fds[i].events = POLLOUT;
		else if (!clientDataMap[client_socket].chunk.empty())
		{
			size_t chunkSize = clientDataMap[client_socket].chunk.size();
			clientDataMap[client_socket].chunk.copy(buffer, chunkSize);
			handlePostRequest(client_socket, buffer, 0, boundary);
		}
		else if (this->clients[client_socket].getIsBinary())
		{
			BodyMap[client_socket].ParseBody("", "", this->clients[client_socket]);
			setClientWritable(client_socket);
		}
		return;
	}

	if (this->clients[client_socket].return_anyway || this->clients[client_socket].getMethod() != POST)
		fds[i].events = POLLOUT;
	else {
		if (bytes_received == -1)
		{
			fds[getClientIndex(client_socket)].events = POLLOUT;
			return;
		}
		handlePostRequest(client_socket, buffer, bytes_received, boundary);
	}
}

std::vector<int> WebServ::getListeners() const
{
	return this->listeners;
}
