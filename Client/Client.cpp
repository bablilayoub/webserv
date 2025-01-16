/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/16 10:46:08 by abablil          ###   ########.fr       */
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

std::string getMimeType(const std::string &path)
{
	static std::map<std::string, std::string> mimeTypes;

	if (mimeTypes.empty())
	{
		mimeTypes[".html"] = "text/html";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".json"] = "application/json";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".webm"] = "video/webm";
		mimeTypes[".ogg"] = "video/ogg";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".wav"] = "audio/wav";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".xml"] = "application/xml";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".zip"] = "application/zip";
	}

	size_t dotPos = path.find('.');
	if (dotPos != std::string::npos)
	{
		std::string ext = path.substr(dotPos);
		if (mimeTypes.count(ext))
			return mimeTypes.at(ext);
	}

	return "text/html";
}

std::string Client::loadFile(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filePath);

	std::stringstream buffer;
	std::string content;

	buffer << file.rdbuf();

	content = buffer.str();
	file.close();
	return content;
}

std::string Client::getErrorPagePath(int errorCode)
{
	std::string error_page_path;
	error_page_path.clear();
	for (std::vector<Server>::iterator serverIt = this->config->servers.begin(); serverIt != this->config->servers.end(); ++serverIt)
	{
		if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->ip) == serverIt->server_names.end() || serverIt->listen_port != this->port)
			continue;
		std::map<int, std::string>::iterator it = serverIt->error_pages.find(errorCode);
		if (it != serverIt->error_pages.end())
			error_page_path = serverIt->root_folder + "/" + it->second;
	}
	return error_page_path;
}

std::string Client::loadFiles(const std::string &directory)
{
	DIR *dir = opendir(directory.c_str());
	if (dir == NULL)
		return this->loadErrorPage(this->getErrorPagePath(404), 404);

	struct dirent *entry;
	struct stat entryStat;
	std::string fileListHTML =
		"<html>"
		"<head>"
		"<style>"
		"body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; }"
		"h1 { text-align: center; color: #333; padding: 20px; background-color: #007BFF; color: #fff; margin: 0; }"
		"table { border-collapse: collapse; margin: 20px auto; max-width: 800px; width: 90%; background-color: #ffffff }"
		"th, td { padding: 10px; border: 1px solid #ddd; text-align: left; }"
		"th { background-color: #007BFF; color: #fff; }"
		"tr:hover { background-color: #f1f1f1; }"
		"</style>"
		"</head>"
		"<body>"
		"<h1>Directory Listing</h1>"
		"<table>"
		"<tr><th>Name</th><th>Type</th></tr>";

	while ((entry = readdir(dir)) != NULL)
	{
		// Skip "." and ".." entries
		if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
			continue;

		std::string entryPath = directory + "/" + entry->d_name;

		// Get file status
		if (stat(entryPath.c_str(), &entryStat) != 0)
			continue;

		// Determine file type
		std::string fileType = S_ISDIR(entryStat.st_mode) ? "Directory" : "File";

		// Add row to the table
		fileListHTML += "<tr>";
		fileListHTML += "<td><a href='/";
		fileListHTML += entry->d_name;
		fileListHTML += "'>" + std::string(entry->d_name) + "</a></td>";
		fileListHTML += "<td>" + fileType + "</td>";
		fileListHTML += "</tr>";
	}

	fileListHTML += "</table></body></html>";
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
				response->content = this->loadErrorPage(this->getErrorPagePath(405), 405); // Method Not Allowed
				response->statusCode = 405;
				return;
			}

			// Check for index file or autoindex functionality
			if (!location.index.empty())
			{
				std::string indexPath = location.root_folder + "/" + location.index;

				if (!fileExists(indexPath))
				{
					response->content = this->loadErrorPage(this->getErrorPagePath(404), 404); // Not Found
					response->statusCode = 404;
					return;
				}

				response->content = this->loadFile(indexPath);
				response->statusCode = 200;
				return;
			}

			if (location.autoindex)
			{
				std::string defaultFilePath = location.root_folder + "/" + location.index;

				if (location.index.empty())
				{
					std::string fullPath = location.root_folder + locIt->first;
					if (!isDirectory(fullPath))
					{
						response->content = this->loadErrorPage(this->getErrorPagePath(404), 404); // Not Found
						response->statusCode = 404;
						return;
					}
					// Return the directory listing if it's a directory
					response->content = loadFiles(fullPath);
					response->statusCode = 200;
					return;
				}

				response->content = this->loadFile(defaultFilePath);
				response->statusCode = 200;
				return;
			}

			if (location.index.empty())
			{
				response->content = this->loadErrorPage(this->getErrorPagePath(404), 404);
				response->statusCode = 404;
				return;
			}
		}
	}

	// Page not found if no matching location or server
	response->content = this->loadErrorPage(this->getErrorPagePath(404), 404);
	response->statusCode = 404;
}

void Client::generateResponse()
{
	struct Response response;

	this->checkConfigs(&response);

	// Determine MIME type based on path
	std::string mimeType = getMimeType(this->path);

	this->response =
		"HTTP/1.1 " + std::to_string(response.statusCode) + " " + this->statusCodes[response.statusCode] + "\r\n" +
		"Content-Type: " + mimeType + "\r\n" +
		"Content-Length: " + std::to_string(response.content.size()) + "\r\n" +
		"Connection: close\r\n"
		"\r\n" +
		response.content;
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
	this->isChunked = false;
	this->isContentLenght = false;
}

void Client::parse(const std::string &request)
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
			size_t transferEncoding = line.find(TRANSFER_ENCODING);

			if (boundaryPos != std::string::npos)
			{
				boundaryPos += std::string(BOUNDARY_PREFIX).length();
				this->boundary = line.substr(boundaryPos, line.size() - 1);
			}
			else if (contentLengthPos != std::string::npos)
			{
				contentLengthPos += std::string(CONTENT_LENGTH_PREFIX).length();
				this->content_length = std::atof(line.substr(contentLengthPos).c_str());
				this->isContentLenght = true;
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
			else if (transferEncoding != std::string::npos)
				this->isChunked = true;
		}
	}

	if (this->firstChunk)
		this->firstChunk = false;
	else
		this->body = request;

	// if (this->method != METHOD_GET)
	// 	BodyMap[this->clientFd].ParseBody(this->body, this->boundary);

	this->generateResponse();
}

const std::string &Client::getResponse() const { return this->response; }

const std::string &Client::getBody() const { return this->body; }

const std::string &Client::getBoundary() const { return this->boundary; }

const std::map<std::string, std::string> &Client::getHeaders() const { return this->headers; }

const int &Client::getContentLength() const { return this->content_length; }

const std::string &Client::getMethod() const { return this->method; }

const bool &Client::getIsChunked() const { return this->isChunked; }

const bool &Client::getIsContentLenght() const { return this->isContentLenght; }
