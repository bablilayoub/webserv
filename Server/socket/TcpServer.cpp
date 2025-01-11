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
        if (new_socket == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return 1;
        else if (new_socket == -1)
            return 2;
        AddClientSocket(poll_fds_vec, new_socket);
    }
    return 0;
}

void TcpServer::handle_clietns(std::vector<pollfd> &poll_fds_vec, size_t i)
{
    char buffer[50];
    int client_socket;
    std::string chunk = "";
    client_socket = poll_fds_vec[i].fd;
    ssize_t bytes_read = -1;

    while (true)
    {

        bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            chunk += buffer;
        }
        if (chunk.length() == MAX_BYTES_TO_SEND - 1 || bytes_read == 0)
        {
            std::cout << chunk << "***************" << std::endl;
            if (bytes_read == 0)
                close(client_socket);
            // poll_fds_vec.erase(poll_fds_vec.begin() + i);
            // --i;
            // send(client_socket, chunk.c_str(), chunk.length(), 0);
            break;
        }
        // else if (bytes_read == -1)
        //     break;
    }
}

int TcpServer::handleIncomingConnections()
{
    std::vector<pollfd> poll_fds_vec;
    AddClientSocket(poll_fds_vec, this->listener);
    pollfd *raw_array = &poll_fds_vec[0];

    while (true)
    {
        int ret = poll(raw_array, poll_fds_vec.size(), 0);
        if (ret == -1)
            break;
        for (size_t i = 0; i < poll_fds_vec.size(); i++)
        {
            if (poll_fds_vec[i].revents & POLLIN)
            {
                int res = accept_IncomingConnection(poll_fds_vec, i);
                if (res != 0)
                {
                    if (res == 1)
                        continue;
                    break;
                }
            }
            else if (poll_fds_vec.size() > 1)
                handle_clietns(poll_fds_vec, i);
        }
    }
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
    // Client new_client(client_socket);
    // clients[client_socket] = new_client;
}