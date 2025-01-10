#include "TcpServer.hpp"

TcpServer::TcpServer() : isNonBlocking(true) {}

void TcpServer::initializeServer(const int port)
{
    int opt;

    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
        throw std::runtime_error("Socket creation failed");
    opt = 1;
    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (isNonBlocking)
        this->setNonBlockingMode(this->sockfd);
    this->socketConfig(PORT);
    if (bind(this->sockfd, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
    {
        close(this->sockfd);
        throw std::runtime_error("bind failed");
    }

    if (listen(this->sockfd, MAX_CLIENTS) == -1)
    {
        close(this->sockfd);
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

int TcpServer::handleIncomingConnections()
{
    char buffer[1024];
    int addrlen, new_socket, client_socket;
    addrlen = sizeof(this->serverAddress);
    std::vector<pollfd> poll_fds_vec;
    AddClientSocket(poll_fds_vec, this->sockfd);
    pollfd *raw_array = &poll_fds_vec[0];

    while (true)
    {
        int ret = poll(raw_array, poll_fds_vec.size(), 100);
        std::cout << "jello\n";
        if (ret == -1)
        {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds_vec.size(); ++i)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                if (poll_fds_vec[i].fd == this->sockfd)
                {
                    new_socket = accept(this->sockfd, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
                    if (new_socket == -1)
                    {
                        std::cerr << "Accept failed" << std::endl;
                        continue;
                    }
                    std::cout << "New client connected!" << std::endl;
                    setNonBlockingMode(new_socket);
                    AddClientSocket(poll_fds_vec, new_socket);
                }
            }
            else
            {
                client_socket = poll_fds_vec[i].fd;
                ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
                if (bytes_read == -1)
                    std::cerr << "Read failed" << std::endl;
                else if (bytes_read == 0)
                {
                    std::cout << "Client disconnected!" << std::endl;
                    close(client_socket);
                    poll_fds_vec.erase(poll_fds_vec.begin() + i);
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    std::cout << "Received message: " << buffer << std::endl;
                    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
                    send(client_socket, response.c_str(), response.length(), 0);
                }
            }
        }
    }
    closeFds(poll_fds_vec);
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
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        close(socket);
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

void TcpServer::AddClientSocket(std::vector<pollfd> &poll_fds_vec, int client_socket)
{
    struct pollfd pfd;

    pfd.fd = client_socket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    poll_fds_vec.push_back(pfd);
}