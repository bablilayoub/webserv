#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

void handle_client(int client_socket)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	// Read the client's request
	int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
	if (bytes_read < 0)
	{
		std::cerr << "Error reading from client socket." << std::endl;
		close(client_socket);
		return;
	}

	std::cout << buffer << std::endl;

	// Parse the HTTP request
	std::istringstream request_stream(buffer);
	std::string method, path, protocol;
	request_stream >> method >> path >> protocol;

	// Handle GET and POST requests
	std::string response;
	if (method == "GET")
	{
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nYou sent a GET request to " + path + "\n";
	}
	else if (method == "POST")
	{
		// Read the body of the POST request
		std::string line;
		while (std::getline(request_stream, line) && line != "\r")
		{
		}
		std::string body;
		std::getline(request_stream, body);

		response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nYou sent a POST request to " + path + " with body: " + body + "\n";
	}
	else
	{
		response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\nMethod Not Allowed\n";
	}

	// Send the response
	send(client_socket, response.c_str(), response.size(), 0);

	// Close the client socket
	close(client_socket);
}

int main()
{
	// Create a socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		std::cerr << "Failed to create socket." << std::endl;
		return 1;
	}

	// Bind the socket to the port
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	int opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "Failed to bind socket." << std::endl;
		close(server_socket);
		return 1;
	}

	// Start listening for connections
	if (listen(server_socket, 10) < 0)
	{
		std::cerr << "Failed to listen on socket." << std::endl;
		close(server_socket);
		return 1;
	}

	std::cout << "Server is running on port " << PORT << std::endl;

	// Handle incoming connections
	while (true)
	{
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
		if (client_socket < 0)
		{
			std::cerr << "Failed to accept client connection." << std::endl;
			continue;
		}

		// Handle the client in a separate thread
		std::thread(handle_client, client_socket).detach();
	}

	close(server_socket);
	return 0;
}