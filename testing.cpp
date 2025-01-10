#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <cstring>

#define PORT 8080
#define MAX_CLIENTS 10

// Function to set the socket to non-blocking mode
void setNonBlocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    int sockfd, new_socket, client_socket;
    struct sockaddr_in serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    struct pollfd pfd;
    std::vector<pollfd> poll_fds_vec; // Vector to hold pollfd structures
    char buffer[1024];

    // Step 1: Create listening socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Step 2: Set the socket to non-blocking mode
    setNonBlocking(sockfd);

    // Step 3: Set up server address and bind
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Binding failed" << std::endl;
        close(sockfd);
        return -1;
    }

    // Step 4: Start listening on the socket
    if (listen(sockfd, MAX_CLIENTS) == -1)
    {
        std::cerr << "Listen failed" << std::endl;
        close(sockfd);
        return -1;
    }

    // Add the listening socket to the pollfd list
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    poll_fds_vec.push_back(pfd);
    pollfd *raw_array = &poll_fds_vec[0];

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while (true)
    {
        // Step 5: Call poll to wait for events
        int ret = poll(raw_array, poll_fds_vec.size(), -1); // Wait indefinitely for events
        if (ret == -1)
        {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        // Step 6: Check events
        for (size_t i = 0; i < poll_fds_vec.size(); ++i)
        {
            // If the listening socket is ready to accept a new connection
            if (poll_fds_vec[i].revents & POLLIN)
            {
                if (poll_fds_vec[i].fd == sockfd)
                {
                    // Accept new connection
                    new_socket = accept(sockfd, (struct sockaddr *)&serverAddr, &addrlen);
                    if (new_socket == -1)
                    {
                        std::cerr << "Accept failed" << std::endl;
                        continue;
                    }

                    std::cout << "New client connected!" << std::endl;

                    // Set the new socket to non-blocking mode
                    setNonBlocking(new_socket);

                    // Add the new client socket to the pollfd list to monitor for data
                    poll_fds_vec.push_back({new_socket, POLLIN, 0});
                }
                else
                {
                    // Read data from an existing client
                    client_socket = poll_fds_vec[i].fd;
                    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
                    if (bytes_read == -1)
                    {
                        std::cerr << "Read failed" << std::endl;
                    }
                    else if (bytes_read == 0)
                    {
                        std::cout << "Client disconnected!" << std::endl;
                        close(client_socket);
                        poll_fds_vec.erase(poll_fds_vec.begin() + i); // Remove the client socket from the pollfd list
                    }
                    else
                    {
                        buffer[bytes_read] = '\0'; // Null-terminate the buffer
                        std::cout << "Received message: " << buffer << std::endl;

                        // Send a simple response
                        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
                        send(client_socket, response.c_str(), response.length(), 0);
                    }
                }
            }
        }
    }

    // Close all poll_fds_vec
    for (const auto &pfd : poll_fds_vec)
    {
        close(pfd.fd);
    }

    return 0;
}
