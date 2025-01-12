#include "TcpServer.hpp"

TcpServer::TcpServer() : isNonBlocking(true) {}

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

    if (listen(this->listener, MAX_CLIENTS) == -1)
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

int TcpServer::accept_IncomingConnection(std::vector<pollfd> &poll_fds_vec, size_t i)
{
    int addrlen, new_socket;
    addrlen = sizeof(this->serverAddress);
    if (poll_fds_vec[i].fd == this->listener)
    {
        new_socket = accept(this->listener, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
        // std::cout << "accept_IncomingConnection with client fd => " << new_socket << std::endl;
        if (new_socket == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return 1;
            return 2;
        }
        // setNonBlockingMode(new_socket);
        AddClientSocket(poll_fds_vec, new_socket);
        // std::cout << "client " << new_socket << "is added" << std::endl;
    }
    return 0;
}

size_t findContentLength(int client_socket)
{
    ssize_t bytes_received;
    char buffer[MAX_BYTES_TO_SEND];
    std::string request;
    while ((bytes_received = recv(client_socket, buffer, MAX_BYTES_TO_SEND - 1, MSG_PEEK)) > 0)
    {
        request.append(buffer, bytes_received);
        size_t pos = request.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            std::string header = request.substr(0, pos);
            size_t content_length_pos = header.find("Content-Length: ");
            if (content_length_pos != std::string::npos)
            {
                size_t content_length_end = header.find("\r\n", content_length_pos);
                std::string content_length_str = header.substr(content_length_pos + 16, content_length_end - content_length_pos - 16);
                return std::stoi(content_length_str);
            }
            else
                return 0;
        }
    }
    return 0;
}

void TcpServer::handle_clients(std::vector<pollfd> &poll_fds_vec, size_t i)
{

    int client_socket;
    ssize_t bytes_received;
    char buffer[MAX_BYTES_TO_SEND];
    std::string chunk = "";
    client_socket = poll_fds_vec[i].fd;
    bytes_received = -1;

    while (true)
    {
        bytes_received = recv(client_socket, buffer, MAX_BYTES_TO_SEND - 1, 0);
        if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "EAGAIN or EWOULDBLOCK" << std::endl;
                break;
            }
            else
            {
                std::cerr << "Failed to receive data from client" << std::endl;
                close(client_socket);
                poll_fds_vec.erase(poll_fds_vec.begin() + i);
                --i;
                return;
            }
        }
        else
        {
            if (bytes_received == 0)
            {
                if (!chunk.empty())
                {
                    std::cout << "last data from client: " << chunk.length() << std::endl;
                    chunk.clear();
                }
                std::cout << "Client disconnected" << std::endl;
                std::string response =
                    "HTTP/1.1 200 OK\n"
                    "Content-Type: text/plain\n"
                    "Content-Length: 13\n"
                    "Connection: close\n"
                    "\n"
                    "Hello, world!";

                send(client_socket, response.c_str(), response.length(), 0);
                close(client_socket);
                poll_fds_vec.erase(poll_fds_vec.begin() + i);
                --i;
                break;
            }

            buffer[bytes_received] = '\0';
            chunk.append(buffer, bytes_received);

            if (chunk.length() >= MAX_BYTES_TO_SEND)
            {
                std::cout << "Received data from client: " << chunk.length() << std::endl;
                chunk.clear();
            }
        }
    }
}

int TcpServer::handleIncomingConnections()
{
    std::vector<pollfd> poll_fds_vec;
    AddClientSocket(poll_fds_vec, this->listener);
    // pollfd *raw_array = &poll_fds_vec[0];

    while (true)
    {
        int ret = poll(poll_fds_vec.data(), poll_fds_vec.size(), 1000);
        if (ret == -1)
        {
            std::cout << "poll failed" << std::endl;
            break;
        }
        for (size_t i = 0; i < poll_fds_vec.size(); i++)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                if (poll_fds_vec[i].fd == this->listener)
                {
                    int res = accept_IncomingConnection(poll_fds_vec, i);
                    if (res == 0 || res == 2)
                        break;
                    else
                        continue;
                }
                else
                    handle_clients(poll_fds_vec, i);
            }
        }
    }
    // closeFds(poll_fds_vec);
    return 1;
}

void TcpServer::closeFds(std::vector<pollfd> &poll_fds_vec)
{

    for (size_t i = 0; i < poll_fds_vec.size(); ++i)
        close(poll_fds_vec[i].fd);
}
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

void TcpServer::AddClientSocket(std::vector<pollfd> &poll_fds_vec, int socket)
{

    struct pollfd pfd;
    pfd.fd = socket;
    pfd.events = POLLIN;
    // pfd.revents = 0;
    poll_fds_vec.push_back(pfd);
    // Client new_client(client_socket);
    // clients[client_socket] = new_client;
}