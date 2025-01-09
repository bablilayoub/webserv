#include "TcpServer.hpp"

TcpServer::TcpServer(int port, bool nonBlocking = false) : isNonBlocking(nonBlocking)
{
    int opt;

    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
        throw std::runtime_error("Socket creation failed");
    opt = 1;
    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (isNonBlocking)
        this->setNonBlockingMode();

    memset(&this->serverAddress, 0, sizeof(this->serverAddress));
    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_port = htons(port);
    this->serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(this->sockfd, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
    {
        close(this->sockfd);
        throw std::runtime_error("bind failed");
    }

    if (listen(this->sockfd, 5) == -1)
    {
        close(this->sockfd);
        throw std::runtime_error("Listening to server failed");
    }
    std::cout << "Server is listening on port " << port << std::endl;
}

int TcpServer::handleIncomingConnection()
{
    int addrlen, new_socket;
    addrlen = sizeof(this->serverAddress);

    while (true)
    {
        new_socket = accept(this->sockfd, (struct sockaddr *)&this->serverAddress, (socklen_t *)&addrlen);
        if (new_socket == -1) {
            std::cerr << "Accept failed" << std::endl;
            close(this->sockfd);
            continue;
        }
        std::cout << "New client connected!" << std::endl;
        char buffer[1024] = {0};
        read(new_socket, buffer, sizeof(buffer));
        std::cout << "Received message: " << buffer << std::endl;
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        send(new_socket, response.c_str(), strlen(response.c_str()), 0);
        close(new_socket);
    }
}

void TcpServer::setNonBlockingMode()
{
    int flags = fcntl(this->sockfd, F_GETFL, 0);
    if (flags == -1) {
        close(this->sockfd);
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(this->sockfd);
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}
