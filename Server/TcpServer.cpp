#include "TcpServer.hpp"

TcpServer::TcpServer(Config* config) : isNonBlocking(true), config(config) {}

void TcpServer::initializeServer(const int port)
{
    int opt;

    this->listener = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listener == -1)
        throw std::runtime_error("Socket creation failed");
    opt = 1;
    setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (isNonBlocking)
        this->setNonBlockingMode(this->listener);
    this->socketConfig(PORT);
    if (bind(this->listener, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
    {
        close(this->listener);
        throw std::runtime_error("bind failed");
    }

    if (listen(this->listener, SOMAXCONN) == -1)
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

int TcpServer::accept_IncomingConnection()
{
    int addrlen, new_socket;
    addrlen = sizeof(this->serverAddress);
    new_socket = accept(this->listener, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
    if (new_socket == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 1;
        return 2;
    }
    // setNonBlockingMode(new_socket);
    AddClientSocket(new_socket);
    return 0;
}

size_t TcpServer::findContentLength(int client_socket, bool *flag)
{
    *flag = true;
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE];
    std::string request;
    int header_length = 0;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, MSG_PEEK)) > 0)
    {
        // std::cout << "findContentLength" << std::endl;
        request.append(buffer, bytes_received);
        size_t pos = request.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            std::string header = request.substr(0, pos);
            header_length = header.length() + 4;
            size_t content_length_pos = header.find("Content-Length: ");
            if (content_length_pos != std::string::npos)
            {
                size_t content_length_end = header.find("\r\n", content_length_pos);
                std::string content_length_str = header.substr(content_length_pos + 16, content_length_end - content_length_pos - 16);
                return std::stoi(content_length_str) + header_length;
            }
            else
                return 0;
        }
    }
    return 0;
}

std::string getBoundary(int client_socket, bool *flag)
{
    *flag = true;
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE];
    std::string request;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, MSG_PEEK)) > 0)
    {
        request.append(buffer, bytes_received);
        size_t pos = request.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            std::string header = request.substr(0, pos);
            size_t boundary_pos = header.find("boundary=");
            if (boundary_pos != std::string::npos)
            {
                size_t boundary_end = header.find("\r\n", boundary_pos);
                std::string boundary = header.substr(boundary_pos + 9, boundary_end - boundary_pos - 9);
                return "--" + boundary;
            }
            else
                return "";
        }
    }
    return "";
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
        // std::cout << chunk << std::endl;
        this->clients[client_socket].parse(chunk, this->BodyMap);
        chunk.clear();
        std::string response = this->clients[client_socket].getResponse();
        send(client_socket, response.c_str(), response.length(), 0);
        cleanUp(client_socket, i);
    }
}

void TcpServer::sendChunks(int client_socket, char *buffer, ssize_t bytes_received, size_t *i, std::string &boundary)
{

    bool flag = false;
    size_t &wholeContentLength = clientData[*i].wholeContentLength;
    size_t &received_content_length = clientData[*i].received_content_length;
    std::string &chunk = clientData[*i].chunk;

    buffer[bytes_received] = '\0';
    chunk.append(buffer, bytes_received);
    received_content_length += bytes_received;
    std::string boundaryString;

    size_t pos = chunk.find(boundary);
    if ((pos = chunk.find(boundary)) != std::string::npos)
    {
        if (pos != 0)
        {
            // std::cout << chunk.substr(0, pos);
            this->clients[client_socket].parse(chunk.substr(0, pos), this->BodyMap);
            chunk = chunk.substr(pos);
            pos = 0;
        }
        size_t boundaryLength = boundary.length();
        boundaryString = chunk.substr(pos, boundaryLength);
        chunk = chunk.substr(pos + boundary.length());
        size_t pos2;
        if ((pos2 = chunk.find(boundaryString)) != std::string::npos)
        {
            // std::cout << boundaryString + chunk.substr(0, pos2);
            this->clients[client_socket].parse(boundaryString + chunk.substr(0, pos2), this->BodyMap);
            chunk = chunk.substr(pos2);
            fileReachedEnd(chunk, client_socket, received_content_length, wholeContentLength, i);
            return;
        }
        else
        {
            flag = true;
            // std::cout << "--------- chunk3 ---------" << std::endl;
        }
    }
    this->clients[client_socket].parse((flag ? boundaryString : "") + chunk, this->BodyMap);
    fileReachedEnd(chunk, client_socket, received_content_length, wholeContentLength, i);
    chunk.clear();
}

void TcpServer::handle_clients(size_t *i)
{
    int client_socket;
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE];

    client_socket = poll_fds_vec[*i].fd;
    std::string &boundary = clientData[*i].boundary;

    if (!clientData[*i].boundary_set)
        boundary = getBoundary(client_socket, &clientData[*i].boundary_set);
    if (!clientData[*i].length_set)
        clientData[*i].wholeContentLength = findContentLength(client_socket, &clientData[*i].length_set);

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
        sendChunks(client_socket, buffer, bytes_received, i, boundary);
}

int TcpServer::handleIncomingConnections()
{
    AddClientSocket(this->listener);
    while (true)
    {
        // std::cout << "handleIncomingConnections" << std::endl;
        int ret = poll(poll_fds_vec.data(), poll_fds_vec.size(), TIME_OUT);
        if (ret == -1)
        {
            std::cout << "poll failed" << std::endl;
            break;
        }
        if (ret == 0)
            continue;
        for (size_t i = 0; i < poll_fds_vec.size(); i++)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                if (poll_fds_vec[i].fd == this->listener)
                {
                    int res = accept_IncomingConnection();
                    if (res == 0 || res == 2)
                        break;
                    else
                        continue;
                }
                else
                    handle_clients(&i);
            }
        }
    }
    // closeFds(clientData.pfd);
    return 1;
}

// void TcpServer::closeFds(std::vector<pollfd> &poll_fds_vec)
// {

//     for (size_t i = 0; i < poll_fds_vec.size(); ++i)
//         close(poll_fds_vec[i].fd);
// }

void TcpServer::setNonBlockingMode(int socket)
{
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

void TcpServer::AddClientSocket(int socket)
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
    pfd.events = POLLIN;
    poll_fds_vec.push_back(pfd);
}