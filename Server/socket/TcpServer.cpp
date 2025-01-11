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

int TcpServer::handleIncomingConnections()
{
    char buffer[200000];
    int addrlen, new_socket, client_socket;
    addrlen = sizeof(this->serverAddress);
    std::vector<pollfd> poll_fds_vec;
    AddClientSocket(poll_fds_vec, this->listener);
    pollfd *raw_array = &poll_fds_vec[0];
    while (true)
    {
        int ret = poll(raw_array, poll_fds_vec.size(), 200);
        if (ret == -1)
        {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            break;
        }
        // else if (ret == 0)
        //     std::cout << "Timeout expired" << std::endl;

        std::cout << (poll_fds_vec[0].revents) << std::endl;
        for (size_t i = 0; i < poll_fds_vec.size(); i++)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                if (poll_fds_vec[i].fd == this->listener)
                {
                    // std::cout << "connction accepted" << std::endl;
                    new_socket = accept(this->listener, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
                    if (new_socket == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        std::cerr << "accept => try later" << std::endl;
                        continue;
                    }
                    else if (new_socket == -1)
                        std::cerr << "Accept failed" << std::endl;
                    setNonBlockingMode(new_socket);
                    std::cout << "****************" << new_socket << std::endl;
                    AddClientSocket(poll_fds_vec, new_socket);
                }
                else
                {
                    std::cout << "accessed to read" << std::endl;
                    client_socket = poll_fds_vec[i].fd;
                    ssize_t bytes_read = -1;
                    std::string chunk;
                    while (true)
                    {
                        bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
                        if (bytes_read > 0)
                        {
                            buffer[bytes_read] = '\0';
                            chunk += buffer;
                        }
                        if (chunk.length() == MAX_BYTES_TO_SEND || bytes_read == 0)
                        {
                            "call bablils fun";
                            break;
                        }
                    }

                    if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        std::cerr << "read => try later" << std::endl;
                        continue;
                    }
                    else if (bytes_read == -1)
                    {
                        std::cerr << "Read failed: " << strerror(errno) << std::endl;
                        close(client_socket); // Handle fatal error
                        poll_fds_vec.erase(poll_fds_vec.begin() + i);
                        --i;
                    }
                    else
                    {
                        buffer[bytes_read] = '\0';
                        std::cout << buffer << std::endl;
                        // call bablil's function
                        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello frida!";
                        send(client_socket, response.c_str(), response.length(), 0);
                    }
                }
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

void TcpServer::AddClientSocket(std::vector<pollfd> &poll_fds_vec, int client_socket)
{
    struct pollfd pfd;

    pfd.fd = client_socket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    poll_fds_vec.push_back(pfd);
}