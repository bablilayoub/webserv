#include "TcpServer.hpp"
// TcpServer::TcpServer() : TcpServer(config) {}

TcpServer::TcpServer(Config *config) : isNonBlocking(true), config(config) {}

void TcpServer::initializeServer(const int port)
{

    this->listener = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listener < 0)
        throw std::runtime_error("Socket creation failed");

    int optval = 1;
    if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        close(this->listener);
        throw std::runtime_error("setsockopt failed");
    }

    if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        close(this->listener);
        throw std::runtime_error("setsockopt failed");
    }

    if (isNonBlocking)
        this->setNonBlockingMode(this->listener);
    this->socketConfig(port);

    if (bind(this->listener, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) < 0)
    {
        close(this->listener);
        throw std::runtime_error("bind failed");
    }

    if (listen(this->listener, SOMAXCONN) < 0)
    {
        close(this->listener);
        throw std::runtime_error("Listening to server failed");
    }
    std::cout << "Server is listening on port " << port << std::endl;
}

void TcpServer::socketConfig(const int port)
{
    memset(&this->serverAddress, 0, sizeof(this->serverAddress));
    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_port = htons(port);
    this->serverAddress.sin_addr.s_addr = INADDR_ANY;
};

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

void TcpServer::getHeaderData(int client_socket, bool *flag, size_t *i, std::string &boundary)
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
            clientData[*i].header_length = header.length();
            break;
        }
    }
    this->clients[client_socket].parse(header);
    clientData[*i].wholeContentLength = this->clients[client_socket].getContentLength() + clientData[*i].header_length;
    boundary = getBoundary(header);
}

void TcpServer::cleanUp(int client_socket, size_t *i)
{
    this->clients.erase(client_socket);
    this->BodyMap.erase(client_socket);
    close(client_socket);
    clientData.erase(clientData.begin() + *i);
    poll_fds_vec.erase(poll_fds_vec.begin() + *i);
    *i -= 1;
}

void TcpServer::fileReachedEnd(std::string &chunk, int client_socket, size_t &received_content_length, size_t &wholeContentLength, size_t *i)
{
    if (received_content_length >= wholeContentLength)
    {
        BodyMap[client_socket].ParseBody(chunk, clientData[*i].boundary, clients[client_socket].getUploadDir());
        chunk.clear();
        std::string response = this->clients[client_socket].getResponse();
        send(client_socket, response.c_str(), response.length(), 0);
        cleanUp(client_socket, i);
    }
}

void TcpServer::parseIfContentLength(int client_socket, std::string &boundary, std::string &chunk, size_t *i, size_t &received_content_length, size_t &wholeContentLength)
{
    bool flag = false;
    std::string boundaryString;

    size_t pos;
    if ((pos = chunk.find(boundary)) != std::string::npos)
    {
        if (pos != 0)
        {
            BodyMap[client_socket].ParseBody(chunk.substr(0, pos), boundary, clients[client_socket].getUploadDir());
            chunk = chunk.substr(pos);
            pos = 0;
        }
        size_t boundaryLength = boundary.length();
        boundaryString = chunk.substr(pos, boundaryLength);
        chunk = chunk.substr(pos + boundaryLength);
        size_t pos2;
        if ((pos2 = chunk.find(boundaryString)) != std::string::npos)
        {
            BodyMap[client_socket].ParseBody(boundaryString + chunk.substr(0, pos2), boundary, clients[client_socket].getUploadDir());
            chunk = chunk.substr(pos2);
            fileReachedEnd(chunk, client_socket, received_content_length, wholeContentLength, i);
            return;
        }
        else
            flag = true;
    }
    BodyMap[client_socket].ParseBody((flag ? boundaryString : "") + chunk, boundary, clients[client_socket].getUploadDir());
    fileReachedEnd(chunk, client_socket, received_content_length, wholeContentLength, i);
    chunk.clear();
}

void TcpServer::handlePostRequest(int client_socket, char *buffer, ssize_t bytes_received, size_t *i, std::string &boundary)
{

    size_t &wholeContentLength = clientData[*i].wholeContentLength;
    size_t &received_content_length = clientData[*i].received_content_length;
    std::string &chunk = clientData[*i].chunk;

    buffer[bytes_received] = '\0';
    chunk.append(buffer, bytes_received);
    received_content_length += bytes_received;

    if (!clientData[*i].removeHeader)
    {
        chunk = chunk.substr(clientData[*i].header_length);
        clientData[*i].removeHeader = true;
    }

    if (this->clients[client_socket].getIsContentLenght())
        parseIfContentLength(client_socket, boundary, chunk, i, received_content_length, wholeContentLength);
    else if (this->clients[client_socket].getIsChunked())
    {
        // parseIfChunked();
    }
    else
    {
        std::string response = this->clients[client_socket].getResponse();
        send(client_socket, response.c_str(), response.length(), 0);
        cleanUp(client_socket, i);
    }
}

void TcpServer::handleClientsRequest(int client_socket, size_t *i)
{
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE];
    std::string &boundary = clientData[*i].boundary;

    if (!clientData[*i].headerDataSet)
        getHeaderData(client_socket, &clientData[*i].headerDataSet, i, boundary);

    if (this->clients[client_socket].getMethod() != POST)
    {
        std::string response = this->clients[client_socket].getResponse();
        send(client_socket, response.c_str(), response.length(), 0);
        cleanUp(client_socket, i);
    }
    else
    {
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == 0)
        {
            std::cout << "Client disconnected" << std::endl;
            cleanUp(client_socket, i);
            return;
        }
        else if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                std::cout << "EAGAIN or EWOULDBLOCK" << std::endl;
            else
            {
                std::cerr << "Failed to receive data from client" << std::endl;
                cleanUp(client_socket, i);
                return;
            }
        }
        else
            handlePostRequest(client_socket, buffer, bytes_received, i, boundary);
    }
}

int TcpServer::handleIncomingConnections()
{
    // AddClientSocket(this->listener, POLLIN);
    // while (true)
    // {
        int ret = poll(poll_fds_vec.data(), poll_fds_vec.size(), 0);
        if (ret == -1)
        {
            std::cout << "poll failed" << std::endl;
            return 1;
        }
        // if (ret == 0)
        //     continue;
        if (poll_fds_vec[0].revents & POLLIN)
        {
            int addrlen, new_socket;
            addrlen = sizeof(this->serverAddress);
            new_socket = accept(this->listener, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
            if (new_socket == -1)
            {
                std::cerr << "Accept failed!" << std::endl;
                // continue;
            }
            setNonBlockingMode(new_socket);
            AddClientSocket(new_socket, POLLIN);
        }

        for (size_t i = 1; i < poll_fds_vec.size(); i++)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                int client_socket = poll_fds_vec[i].fd;
                handleClientsRequest(client_socket, &i);
            }
            // if (poll_fds_vec[i].revents & POLLOUT)
            // {
            // }
        }
        // break;
    // }
    // closeFds();
    return 0;
}

void TcpServer::closeFds()
{

    for (size_t i = 0; i < poll_fds_vec.size(); ++i)
        close(poll_fds_vec[i].fd);
}

void TcpServer::setNonBlockingMode(int socket)
{
    // forbidden Macro => F_GETFL
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1)
    {
        close(socket);
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        close(socket);
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

void TcpServer::AddClientSocket(int socket, int event)
{

    ClientData data;
    clientData.push_back(data);

    if (socket != this->listener)
    {
        this->clients[socket] = Client();
        this->clients[socket].setup(socket, this->config);
    }

    struct pollfd pfd;
    pfd.fd = socket;
    pfd.events = event;
    poll_fds_vec.push_back(pfd);
}

int TcpServer::getListner() const
{
    return this->listener;
}