/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/15 16:08:30 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client()
{
	this->firstChunk = true;

	this->statusCodes[200] = "OK";
	this->statusCodes[201] = "Created";
	this->statusCodes[202] = "Accepted";
	this->statusCodes[301] = "Moved Permanently";
	this->statusCodes[302] = "Found";
	this->statusCodes[307] = "Temporary Redirect";
	this->statusCodes[308] = "Permanent Redirect";
	this->statusCodes[400] = "Bad Request";
	this->statusCodes[401] = "Unauthorized";
	this->statusCodes[403] = "Forbidden";
	this->statusCodes[404] = "Not Found";
	this->statusCodes[405] = "Method Not Allowed";
	this->statusCodes[500] = "Internal Server Error";
	this->statusCodes[505] = "HTTP Version Not Supported";
}

void Client::setup(int fd, Config *config)
{
	this->clientFd = fd;
	this->config = config;
}

const std::string &Client::getResponse() const { return this->response; }

std::string Client::loadErrorPage(const std::string &filePath, int statusCode)
{
	std::stringstream buffer;

	std::string statusMessage = statusCodes.count(statusCode) > 0 ? statusCodes.at(statusCode) : "Unknown Error";

	std::string html =
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"    <meta charset=\"UTF-8\">\n"
		"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"    <title>Error " +
		std::to_string(statusCode) + "</title>\n"
									 "    <style>\n"
									 "        body {\n"
									 "            font-family: Arial, sans-serif;\n"
									 "            text-align: center;\n"
									 "            background-color: #f4f4f9;\n"
									 "            color: #333;\n"
									 "            margin: 0;\n"
									 "            padding: 50px;\n"
									 "        }\n"
									 "        h1 {\n"
									 "            font-size: 3em;\n"
									 "            color: #ff6b6b;\n"
									 "        }\n"
									 "        p {\n"
									 "            font-size: 1.2em;\n"
									 "        }\n"
									 "        a {\n"
									 "            text-decoration: none;\n"
									 "            color: #3498db;\n"
									 "        }\n"
									 "        a:hover {\n"
									 "            text-decoration: underline;\n"
									 "        }\n"
									 "    </style>\n"
									 "</head>\n"
									 "<body>\n"
									 "    <h1>" +
		std::to_string(statusCode) + " - " + statusMessage + "</h1>\n"
															 "    <p>Sorry, the page you are looking for cannot be found.</p>\n"
															 "    <p><a href=\"/\">Return to Home</a></p>\n"
															 "</body>\n"
															 "</html>";

	if (!filePath.empty())
	{
		std::ifstream file(filePath.c_str());
		if (file.is_open())
		{
			buffer << file.rdbuf();
			file.close();
			html = buffer.str();
		}
	}

	return html;
}

bool fileExists(const std::string &path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string &path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

std::string Client::loadFile(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filePath);

	std::stringstream buffer;
	std::string html;

	buffer << file.rdbuf();

	html = buffer.str();
	file.close();
	return html;
}

std::string Client::loadFiles(const std::string& directory) {
    DIR* dir = opendir(directory.c_str());
    if (dir == NULL)
        return this->loadErrorPage("", 404);

    struct dirent* entry;
    std::string fileListHTML = "<html><body><h1>Directory Listing</h1><ul>";

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
            continue;

        fileListHTML += "<li><a href=\"" + directory + "/" + entry->d_name + "\">" + entry->d_name + "</a></li>";
    }

    fileListHTML += "</ul></body></html>";
    closedir(dir);
    return fileListHTML;
}

void Client::checkConfigs(struct Response *response)
{
	// Iterate over servers
	for (std::vector<Server>::const_iterator serverIt = config->servers.begin(); serverIt != config->servers.end(); ++serverIt)
	{
		// Check if the server name and port match
		if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->ip) == serverIt->server_names.end() || serverIt->listen_port != this->port)
			continue;

		// Iterate over locations for the server
		for (std::map<std::string, Location>::const_iterator locIt = serverIt->locations.begin(); locIt != serverIt->locations.end(); ++locIt)
		{
			const Location &location = locIt->second;

			// Check if the path matches the location
			if (locIt->first != this->path)
				continue;

			// Check if the method is accepted for this location
			if (std::find(location.accepted_methods.begin(), location.accepted_methods.end(), this->method) == location.accepted_methods.end())
			{
				response->html = this->loadErrorPage("", 405); // Method Not Allowed
				response->statusCode = 405;
				return;
			}

			// Check for index file or autoindex functionality
			if (!location.index.empty())
			{
				std::string indexPath = location.root_folder + "/" + location.index;

				if (!fileExists(indexPath))
				{
					response->html = this->loadErrorPage("", 404); // Not Found
					response->statusCode = 404;
					return;
				}

				response->html = this->loadFile(indexPath);
				response->statusCode = 200;
				return;
			}

            if (location.autoindex)
            {
                std::string defaultFilePath = location.root_folder + "/" + location.default_file;

                if (location.default_file.empty())
                {
                    std::string fullPath = location.root_folder + locIt->first;
                    if (!isDirectory(fullPath))
                    {
                        response->html = this->loadErrorPage("", 404); // Not Found
                        response->statusCode = 404;
                        return;
                    }
                    // Return the directory listing if it's a directory
                    response->html = loadFiles(fullPath);
                    response->statusCode = 200;
                    return;
                }

                response->html = this->loadFile(defaultFilePath);
                response->statusCode = 200;
                return;
            }


			// Return the file specified in the index
			response->html = this->loadFile(location.root_folder + "/" + location.index);
			response->statusCode = 200;
			return;
		}
	}

	// Page not found if no matching location or server
	response->html = this->loadErrorPage("", 404);
	response->statusCode = 404;
}

void Client::generateResponse()
{
	struct Response response;

	this->checkConfigs(&response);

	this->response =
		"HTTP/1.1 " + std::to_string(response.statusCode) + " " + this->statusCodes[response.statusCode] + "\r\n"
																										   "Content-Type: text/html\r\n"
																										   "Content-Length: " +
		std::to_string(response.html.size()) + "\r\n"
											   "Connection: close\r\n"
											   "\r\n" +
		response.html;
}

void Client::handleFirstLine(std::istringstream &requestStream)
{
	std::string line;
	std::getline(requestStream, line);

	size_t rpos = line.find('\r');
	if (rpos != std::string::npos)
		line = line.substr(0, rpos);

	size_t firstSpace = line.find(' ');
	if (firstSpace == std::string::npos)
		throw std::runtime_error("Invalid request line format");

	this->method = line.substr(0, firstSpace);
	size_t secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
		throw std::runtime_error("Invalid request line format");

	this->path = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
}

void Client::clear()
{
	this->port = 0;
	this->content_length = 0;
	this->ip.clear();
	this->path.clear();
	this->method.clear();
	this->body.clear();
	this->boundary.clear();
	this->headers.clear();
}

void Client::parse(const std::string &request, std::map<int, FileUpload> &BodyMap)
{
	size_t pos = 0;
	size_t endPos = request.find("\r\n\r\n", pos);

	if (endPos != std::string::npos && this->firstChunk)
	{
		this->clear();

		std::string line;
		std::string headers = request.substr(0, endPos);
		std::istringstream headerStream(headers);

		this->handleFirstLine(headerStream);

		while (std::getline(headerStream, line))
		{
			size_t rpos = line.find('\r');
			if (rpos != std::string::npos)
				line = line.substr(0, rpos);

			size_t boundaryPos = line.find(BOUNDARY_PREFIX);
			size_t contentLengthPos = line.find(CONTENT_LENGTH_PREFIX);
			size_t hostPrefixPos = line.find(HOST_PREFIX);
			size_t colonPos = line.find(':');

			if (boundaryPos != std::string::npos)
			{
				boundaryPos += std::string(BOUNDARY_PREFIX).length();
				this->boundary = line.substr(boundaryPos, line.size() - 1);
			}
			else if (contentLengthPos != std::string::npos)
			{
				contentLengthPos += std::string(CONTENT_LENGTH_PREFIX).length();
				this->content_length = std::atof(line.substr(contentLengthPos).c_str());
			}
			else if (hostPrefixPos != std::string::npos)
			{
				size_t colonPos = line.find(':', hostPrefixPos + std::string(HOST_PREFIX).length());
				if (colonPos == std::string::npos)
					throw std::runtime_error("Invalid host format");

				this->ip = line.substr(hostPrefixPos + std::string(HOST_PREFIX).length(),
									   colonPos - (hostPrefixPos + std::string(HOST_PREFIX).length()));
				this->port = std::atof(line.substr(colonPos + 1).c_str());
			}
			else if (colonPos != std::string::npos)
				this->headers[line.substr(0, colonPos)] = line.substr(colonPos + 2);
		}
	}

	if (this->firstChunk)
	{
		this->body = request.substr(endPos + 4);
		this->firstChunk = false;
	}
	else
		this->body = request;

	if (this->method != METHOD_GET)
		BodyMap[this->clientFd].ParseBody(this->body, this->boundary);

	this->generateResponse();
}

const std::string &Client::getBody() const
{
	return this->body;
}

const std::string &Client::getBoundary() const
{
	return this->boundary;
}

const std::map<std::string, std::string> &Client::getHeaders() const
{
	return this->headers;
}

const int &Client::getContentLength() const
{
	return this->content_length;
}
