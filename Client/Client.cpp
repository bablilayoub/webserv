/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/20 12:24:39 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

bool Client::isCGIRequest(const std::string &path)
{
	if (path.substr(path.find_last_of(".") + 1) == "py" || path.substr(path.find_last_of(".") + 1) == "php")
		return true;
	return false;
}

void Client::setErrorResponse(int statusCode)
{
	response.content = loadErrorPage(getErrorPagePath(statusCode), statusCode);
	response.statusCode = statusCode;
}

Server *Client::getServer()
{
	for (std::vector<Server>::iterator serverIt = this->config->servers.begin(); serverIt != this->config->servers.end(); ++serverIt)
	{
		if (std::find(serverIt->ports.begin(), serverIt->ports.end(), this->port) != serverIt->ports.end())
		{
			if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->server_name) != serverIt->server_names.end())
				return &(*serverIt);
		}
	}
	return NULL;
}

Location *Client::getLocation()
{
	Server *server = this->getServer();
	if (server == NULL)
		return NULL;

	for (std::map<std::string, Location>::iterator locIt = server->locations.begin(); locIt != server->locations.end(); ++locIt)
	{
		if (locIt->first == this->path)
			return &locIt->second;
	}
	return NULL;
}

void Client::handleCGIRequest(const std::string &indexPath)
{
	std::string cgiPath;
	if (indexPath.find(".php") != std::string::npos)
		cgiPath = "/Users/alaalalm/Desktop/webserv/Cgi/php-cgi";
	else
		cgiPath = "/Users/alaalalm/Desktop/webserv/Cgi/python-cgi";
	const std::string outPutFile = "/tmp/cgi_out_" + std::to_string(this->clientFd);
	const std::string tempDataFile = "/tmp/cgi_input_" + std::to_string(this->clientFd);

	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Failed to fork the process" << std::endl;
		this->setErrorResponse(500);
		return;
	}

	if (pid == 0)
	{
		this->body = "name=John+Doe&email=johndoe%40example.com";

		std::map<std::string, std::string> env;
		env["GATEWAY_INTERFACE"] = "CGI/1.1";
		env["SCRIPT_FILENAME"] = indexPath;
		env["REQUEST_METHOD"] = method;
		env["CONTENT_TYPE"] = this->content_type;
		env["CONTENT_LENGTH"] = std::to_string(this->body.size());
		env["REDIRECT_STATUS"] = "200";
		env["REMOTE_ADDR"] = this->server_name;
		env["SERVER_PORT"] = std::to_string(port);
		env["HTTP_USER_AGENT"] = "Client ID:" + std::to_string(clientFd);

		char **envp = new char *[env.size() + 1];
		int i = 0;
		for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
		{
			envp[i] = strdup((it->first + "=" + it->second).c_str());
			++i;
		}
		envp[i] = NULL;

		int outFd = open(outPutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (outFd < 0)
		{
			for (int j = 0; j < i; ++j)
				free(envp[j]);
			delete[] envp;
			exit(EXIT_FAILURE);
		}
		dup2(outFd, STDOUT_FILENO);
		close(outFd);

		std::ofstream cgiInput(tempDataFile.c_str());
		if (cgiInput)
			cgiInput << this->body;
		cgiInput.close();

		const char *argv[] = {cgiPath.c_str(), indexPath.c_str(), NULL};

		int inputFd = open(tempDataFile.c_str(), O_RDONLY);
		if (inputFd < 0)
		{
			std::cerr << "Failed to open temp form data file" << std::endl;
			exit(EXIT_FAILURE);
		}
		dup2(inputFd, STDIN_FILENO);
		close(inputFd);

		if (execve(cgiPath.c_str(), const_cast<char **>(argv), envp) == -1)
		{
			for (int j = 0; j < i; ++j)
				free(envp[j]);
			delete[] envp;
			exit(EXIT_FAILURE);
		}
		for (int j = 0; j < i; ++j)
			free(envp[j]);
		delete[] envp;
	}
	else
	{
		int status;
		if (waitpid(pid, &status, 0) == -1)
		{
			this->setErrorResponse(500);
			return;
		}

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			std::ifstream cgiOutput(outPutFile.c_str());
			if (cgiOutput)
			{
				std::stringstream buffer;
				buffer << cgiOutput.rdbuf();

				size_t double_crlf = buffer.str().find("\r\n\r\n");
				if (double_crlf != std::string::npos)
					response.content = buffer.str().substr(double_crlf + 4);
				else
					response.content = buffer.str();
				response.statusCode = 200;
			}
			else
				this->setErrorResponse(500);
		}
		else
			this->setErrorResponse(500);

		unlink(outPutFile.c_str());
	}
}

void Client::setup(int fd, Config *config)
{
	this->clientFd = fd;
	this->config = config;
}

void Client::logRequest(int statusCode)
{
	std::time_t now = std::time(0);
	std::tm *localTime = std::localtime(&now);

	std::string statusColor;
	if (statusCode >= 200 && statusCode < 300)
		statusColor = GREEN;
	else if (statusCode >= 300 && statusCode < 400)
		statusColor = CYAN;
	else if (statusCode >= 400 && statusCode < 500)
		statusColor = YELLOW;
	else
		statusColor = RED;

	std::cout << "[" << (1900 + localTime->tm_year) << "-"
			  << std::setw(2) << std::setfill('0') << (localTime->tm_mon + 1) << "-"
			  << std::setw(2) << std::setfill('0') << localTime->tm_mday << " "
			  << std::setw(2) << std::setfill('0') << localTime->tm_hour << ":"
			  << std::setw(2) << std::setfill('0') << localTime->tm_min << ":"
			  << std::setw(2) << std::setfill('0') << localTime->tm_sec << "] "
			  << std::setfill(' ')
			  << BOLD << "Server: " << BLUE << this->server_name + ":" + std::to_string(this->port) << RESET << " "
			  << BOLD << "Method: " << BLUE << std::setw(8) << std::left << this->method << RESET
			  << BOLD << "Path: " << WHITE << std::setw(30) << this->path << RESET
			  << BOLD << "Status: " << statusColor << statusCode << RESET
			  << std::endl;
}

std::string Client::loadErrorPage(const std::string &filePath, int statusCode)
{
	std::stringstream buffer;

	std::string statusMessage = this->config->statusCodes.count(statusCode) > 0 ? this->config->statusCodes.at(statusCode) : "Unknown Error";

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

bool hasReadPermission(const std::string &path)
{
	return (access(path.c_str(), R_OK) == 0);
}

std::string Client::getMimeType(const std::string &path)
{
	size_t dotPos = path.find('.');
	if (dotPos != std::string::npos)
	{
		std::string ext = path.substr(dotPos);
		if (this->config->mimeTypes.count(ext))
			return this->config->mimeTypes.at(ext);
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
	Server *server = this->getServer();
	std::map<int, std::string>::iterator it = server->error_pages.find(errorCode);
	if (it != server->error_pages.end())
		error_page_path = server->root_folder + "/" + it->second;
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

void Client::checkConfigs()
{
	Server *server = this->getServer();

	if (!server)
	{
		this->setErrorResponse(404);
		return;
	}

	Location *location = this->getLocation();

	if (location)
	{
		if (std::find(location->accepted_methods.begin(), location->accepted_methods.end(), this->method) == location->accepted_methods.end())
		{
			this->setErrorResponse(405);
			return;
		}

		if (!location->index.empty())
		{
			std::string indexPath = location->root_folder + "/" + location->index;

			if (!fileExists(indexPath))
			{
				this->setErrorResponse(404);
				return;
			}

			if (!hasReadPermission(indexPath))
			{
				this->setErrorResponse(403);
				return;
			}

			if (isCGIRequest(indexPath))
			{
				handleCGIRequest(indexPath);
				return;
			}

			response.content = this->loadFile(indexPath);
			response.statusCode = 200;
			return;
		}

		if (location->autoindex)
		{
			std::string defaultFilePath = location->root_folder + "/" + location->index;

			if (location->index.empty())
			{
				std::string fullPath = location->root_folder + this->path;
				if (!isDirectory(fullPath))
				{
					this->setErrorResponse(404);
					return;
				}

				response.content = this->loadFiles(fullPath);
				response.statusCode = 200;
				return;
			}

			response.content = this->loadFile(defaultFilePath);
			response.statusCode = 200;
			return;
		}

		if (location->index.empty())
		{
			this->setErrorResponse(404);
			return;
		}
	}
	
	if (isDirectory(server->root_folder + this->path))
	{
		response.content = this->loadFiles(server->root_folder + this->path);
		response.statusCode = 200;
		return;
	}

	if (fileExists(server->root_folder + this->path))
	{
		response.content = loadFile(server->root_folder + this->path);
		response.statusCode = 200;
		return;
	}

	this->setErrorResponse(404);
}

std::string setHttpHeaders(const std::string &contentType, const std::string &content)
{
	std::string headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: " + contentType + "\r\n";
	headers += "Content-Length: " + std::to_string(content.size()) + "\r\n";
	headers += "Connection: close\r\n";
	headers += "\r\n";
	return headers;
}

void Client::generateResponse()
{
	this->response.content.empty();
	this->response.statusMessage.empty();
	this->response.contentType.empty();
	this->response.statusCode = 200;

	this->checkConfigs();

	logRequest(response.statusCode);

	std::string mimeType = getMimeType(this->path);

	this->responseString =
		setHttpHeaders(mimeType, response.content) +
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
	this->server_name.clear();
	this->path.clear();
	this->method.clear();
	this->body.clear();
	this->boundary.clear();
	this->headers.clear();
	this->upload_dir.empty();
	this->content_type.empty();
	this->isChunked = false;
	this->isContentLenght = false;
	this->isBinary = false;
}

void Client::parse(const std::string &request)
{
	size_t pos = 0;
	size_t endPos = request.find("\r\n\r\n", pos);

	if (endPos != std::string::npos)
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

			size_t contentLengthPos = line.find(CONTENT_LENGTH_PREFIX);
			size_t contentTypePos = line.find(CONTENT_TYPE_PREFIX);
			size_t hostPrefixPos = line.find(HOST_PREFIX);
			size_t colonPos = line.find(':');
			size_t transferEncoding = line.find(TRANSFER_ENCODING);

			if (contentLengthPos != std::string::npos)
			{
				contentLengthPos += std::string(CONTENT_LENGTH_PREFIX).length();
				this->content_length = std::atof(line.substr(contentLengthPos).c_str());
				this->isContentLenght = true;
			}
			else if (contentTypePos != std::string::npos)
			{
				contentTypePos += std::string(CONTENT_TYPE_PREFIX).length();
				this->content_type = line.substr(contentTypePos, line.size() - 1).c_str();
				size_t boundaryPos = this->content_type.find(BOUNDARY_PREFIX);
				if (boundaryPos != std::string::npos)
				{
					boundaryPos += std::string(BOUNDARY_PREFIX).length();
					this->boundary = this->content_type.substr(boundaryPos, this->content_type.size() - 1);
				}
				else if (this->content_type.find("application/x-www-form-urlencoded") == std::string::npos)
					this->isBinary = true;
			}
			else if (hostPrefixPos != std::string::npos)
			{
				size_t colonPos = line.find(':', hostPrefixPos + std::string(HOST_PREFIX).length());
				if (colonPos == std::string::npos)
					throw std::runtime_error("Invalid host format");

				this->server_name = line.substr(hostPrefixPos + std::string(HOST_PREFIX).length(),
												colonPos - (hostPrefixPos + std::string(HOST_PREFIX).length()));
				this->port = std::atof(line.substr(colonPos + 1).c_str());
			}
			else if (colonPos != std::string::npos)
				this->headers[line.substr(0, colonPos)] = line.substr(colonPos + 2);
			else if (transferEncoding != std::string::npos)
				this->isChunked = true;
		}
	}

	for (std::vector<Server>::const_iterator serverIt = config->servers.begin(); serverIt != config->servers.end(); ++serverIt)
	{
		// if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->server_name) == serverIt->server_names.end() || serverIt->listen_port != this->port)
		if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->server_name) == serverIt->server_names.end())
			continue;

		for (std::map<std::string, Location>::const_iterator locIt = serverIt->locations.begin(); locIt != serverIt->locations.end(); ++locIt)
		{
			if (this->path == locIt->first)
				this->upload_dir = locIt->second.upload_dir;
		}
	}

	if (!this->isChunked && !this->isContentLenght && this->method == METHOD_POST)
	{
		response.statusCode = 400;
		logRequest(response.statusCode);

		std::string mimeType = getMimeType(this->path);
		response.content = this->loadErrorPage(this->getErrorPagePath(400), 400);
		this->responseString =
			"HTTP/1.1 " + std::to_string(response.statusCode) + " " + this->config->statusCodes[response.statusCode] + "\r\n" +
			"Content-Type: " + mimeType + "\r\n" +
			"Content-Length: " + std::to_string(response.content.size()) + "\r\n" +
			"Connection: close\r\n"
			"\r\n" +
			response.content;
		return;
	}

	this->generateResponse();
}

const std::string &Client::getResponse() const { return this->responseString; }

const std::string &Client::getBody() const { return this->body; }

const std::string &Client::getBoundary() const { return this->boundary; }

const std::map<std::string, std::string> &Client::getHeaders() const { return this->headers; }

const int &Client::getContentLength() const { return this->content_length; }

const std::string &Client::getMethod() const { return this->method; }

const bool &Client::getIsChunked() const { return this->isChunked; }

const bool &Client::getIsBinary() const { return this->isBinary; }

const bool &Client::getIsContentLenght() const { return this->isContentLenght; }

const std::string &Client::getUploadDir() const { return this->upload_dir; }

const std::string &Client::getContentType() const { return this->content_type; }