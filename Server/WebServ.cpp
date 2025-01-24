#include "WebServ.hpp"

////////////////////////////////////////
///           SERVER SETUP           ///
////////////////////////////////////////

WebServ::WebServ(Config *config) : config(config) {}

int WebServ::init(std::string host, const int port)
{
  int listener = -1;
  while (true)
  {
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET)
    {
      std::cout << "Socket creation failed" << std::endl;
      continue;
    }

    int optval = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == SOCKET_ERROR)
    {
      close(listener);
      std::cout << "setsockopt failed" << std::endl;
      continue;
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == SOCKET_ERROR)
    {
      close(listener);
      std::cout << "setsockopt failed" << std::endl;
      continue;
    }

    this->socketConfig(host, port);
    if (this->setNonBlockingMode(listener) == -1)
    {
      close(listener);
      std::cout << "Failed to set non-blocking mode" << std::endl;
      continue;
    }

    if (bind(listener, (sockaddr *)&this->hint, sizeof(this->hint)) == SOCKET_ERROR)
    {
      close(listener);
      std::cout << "bind failed" << std::endl;
      continue;
    }

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
    {
      close(listener);
      std::cout << "Listening to server failed" << std::endl;
      continue;
    }
    std::cout << "Server is listening on port " << port << std::endl;
    return listener;
  }
}

void WebServ::socketConfig(std::string host, const int port)
{
  memset(&this->hint, 0, sizeof(this->hint));
  this->hint.sin_family = AF_INET;
  this->hint.sin_port = htons(port);
  if (inet_pton(AF_INET, host.c_str(), &this->hint.sin_addr) <= 0)
    std::cerr << "Invalid address: " << host << std::endl; // to remove
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
  uint16_t serversSize = this->config->servers.size();
  std::string host;
  std::vector<int> ports;
  int listener;

  for (size_t i = 0; i < serversSize; i++)
  {
    host = this->config->servers[i].host;
    ports = this->config->servers[i].ports;

    for (size_t j = 0; j < ports.size(); j++)
    {
      // if (std::find(ports.begin(), ports.end(), ports[i]) != ports.end())
      //   continue;
      listener = this->init(host, ports[j]);
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
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      std::cerr << "whohooooo" << std::endl;
    std::cerr << "Accept failed!" << std::endl;
    return -1;
  }
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
    std::remove(("/tmp/cgi_input_" + std::to_string(socket)).c_str());
  }
  struct pollfd pfd;
  pfd.fd = socket;
  pfd.events = event;
  fds.push_back(pfd);
}

void WebServ::handleServersIncomingConnections()
{
  while (true)
  {
    int ret = poll(fds.data(), fds.size(), TIME_OUT);
    if (ret == -1)
    {
      closeFds();
      throw std::runtime_error("Poll failed");
    }
    if (ret == 0)
      continue;

    

    for (size_t i = 0; i < fds.size(); i++)
    {
      if (fds[i].revents & POLLIN)
      {
        if (std::find(listeners.begin(), listeners.end(), fds[i].fd) != listeners.end())
          acceptConnectionsFromListner(fds[i].fd);
        else
          handleClientsRequest(fds[i].fd, i);
      }
      if (fds[i].revents & POLLOUT)
      {
        int client_socket = fds[i].fd;

        this->clients[client_socket].generateResponse();

        if (clients[client_socket].getIsCGI())
          if (!clients[client_socket].checkCGICompletion())
            continue;

        std::string response = this->clients[client_socket].getResponse();
        ssize_t &client_sentBytes = this->clientDataMap[client_socket].sent_bytes;
        ssize_t bytes_sent = send(client_socket, response.c_str() + client_sentBytes,
                                  response.length() - client_sentBytes, 0);
        if (bytes_sent > 0)
        {
          client_sentBytes += bytes_sent;
          if ((size_t)client_sentBytes == response.length())
            cleanUp(client_socket, i);
        }
      }
    }
  }
}

////////////////////////////////////////
///         CLEAR RESOURCES          ///
////////////////////////////////////////

void WebServ::closeFds()
{
  for (size_t i = 0; i < fds.size(); ++i)
    close(fds[i].fd);
}

void WebServ::cleanUp(int client_socket, size_t &i)
{
  this->clients.erase(client_socket);
  this->BodyMap.erase(client_socket);
  this->clientDataMap.erase(client_socket);
  fds.erase(fds.begin() + i);
  i--;
  close(client_socket);
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

void WebServ::getHeaderData(int client_socket, bool *flag, std::string &boundary)
{
  char buffer[BUFFER_SIZE];
  std::string request;
  std::string header;
  ssize_t bytes_received;

  *flag = true;
  while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, MSG_PEEK)) > 0)
  {
    request.append(buffer, bytes_received);
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
      header = request.substr(0, pos + 4);
      this->clientDataMap[client_socket].header_length = header.length();
      break;
    }
  }
  this->clients[client_socket].parse(header);
  this->clientDataMap[client_socket].wcl = this->clients[client_socket].getContentLength() + this->clientDataMap[client_socket].header_length;
  boundary = getBoundary(header);
}

int WebServ::getClientIndex(int client_socket)
{
  for (size_t i = 0; i < fds.size(); ++i)
  {
    if (fds[i].fd == client_socket)
    {
      return i;
    }
  }
  return -1;
}

void WebServ::fileReachedEnd(std::string &chunk, int client_socket, size_t &rcl, size_t &wcl, std::ofstream &cgiInput)
{
  if (rcl >= wcl)
  {
    if (!this->clients[client_socket].getIsCGI()) {
      BodyMap[client_socket].ParseBody(chunk, this->clientDataMap[client_socket].boundary, this->clients[client_socket]);
    }
    cgiInput.close();
    size_t index = getClientIndex(client_socket);
    fds[index].events = POLLOUT;
    chunk.clear();
  }
}

void WebServ::parseFormData(int client_socket, std::string &boundary, std::string &chunk, size_t &rcl, size_t &wcl, std::ofstream &cgiInput)
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
      BodyMap[client_socket].ParseBody(boundaryString + chunk.substr(0, pos2), boundary, this->clients[client_socket]);
      chunk = chunk.substr(pos2);
      fileReachedEnd(chunk, client_socket, rcl, wcl, cgiInput);
      return;
    }
    else
      flag = true;
  }
  BodyMap[client_socket].ParseBody((flag ? boundaryString : "") + chunk, boundary, this->clients[client_socket]);
  chunk.clear();
  fileReachedEnd(chunk, client_socket, rcl, wcl, cgiInput);
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

  if (!this->clientDataMap[client_socket].removeHeader)
  {
    chunk = chunk.substr(this->clientDataMap[client_socket].header_length);
    this->clientDataMap[client_socket].removeHeader = true;
  }

  if (this->clients[client_socket].getIsCGI())
  {
    cgiInput << chunk;
    chunk.clear();
    fileReachedEnd(chunk, client_socket, rcl, wcl, cgiInput);
  }
  else if (this->clients[client_socket].getContentType().find("multipart/form-data;") != std::string::npos)
    parseFormData(client_socket, boundary, chunk, rcl, wcl, cgiInput);
  else if (this->clients[client_socket].getIsBinary())
  {
    BodyMap[client_socket].ParseBody(chunk, "", this->clients[client_socket]);
    chunk.clear();
    fileReachedEnd(chunk, client_socket, rcl, wcl, cgiInput);
  }
}

void WebServ::handleClientsRequest(int client_socket, size_t &i)
{
  ssize_t bytes_received;
  char buffer[BUFFER_SIZE];
  std::string &boundary = this->clientDataMap[client_socket].boundary;

  if (!this->clientDataMap[client_socket].headerDataSet)
    getHeaderData(client_socket, &this->clientDataMap[client_socket].headerDataSet, boundary);

  if (this->clients[client_socket].getMethod() != POST)
    fds[i].events = POLLOUT;
  else
  {
    if (this->clients[client_socket].getContentLength() == 0 || this->clients[client_socket].getContentLength() > 100000000000)
    {
      std::cerr << "Content-Length is invalid" << std::endl;
      fds[i].events = POLLOUT;
      return;
    }
    bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    size_t index = getClientIndex(client_socket);
    if (bytes_received == 0)
    {
      std::cout << "Client disconnected" << std::endl;
      fds[index].events = POLLOUT;
    }
    else if (bytes_received == -1)
    {
      std::cerr << "Failed to receive data from client" << std::endl;
      fds[index].events = POLLOUT;

      // if (errno == EAGAIN || errno == EWOULDBLOCK)
      //   std::cout << "EAGAIN or EWOULDBLOCK" << std::endl;
      // else
      // {
      //   fds[index].events = POLLOUT;
      //   return;
      // }
    }
    else
      handlePostRequest(client_socket, buffer, bytes_received, boundary);
  }
}
