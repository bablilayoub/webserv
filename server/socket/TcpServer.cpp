#include "TcpServer.hpp"

TcpServer::TcpServer()
{
    memset(&this->serverAddress, 0, sizeof(this->serverAddress));
    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_port = htons(PORT);
    this->serverAddress.sin_addr.s_addr = INADDR_ANY;
}

int TcpServer::socketCreationAndBinding()
{
    int opt;

    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
    {
        std::cout << "Socket creation failed" << std::endl;
        return -1;
    }

    opt = 1;
    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(this->sockfd, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
    {
        std::cout << "bind failed" << std::endl;
        close(this->sockfd);
        return -1;
    }
    return 0;
}

int TcpServer::handleIncomingConnection()
{
    int addrlen, new_socket;
    addrlen = sizeof(this->serverAddress);

    if (listen(this->sockfd, 5) == -1)
    {
        std::cout << "Listening to server failed" << std::endl;
        close(this->sockfd);
        return -1;
    }

    while (true)
    {
        new_socket = accept(this->sockfd, (struct sockaddr *)&this->serverAddress, (socklen_t *)&addrlen);
        if (new_socket == -1)
        {
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
    close(this->sockfd);
}