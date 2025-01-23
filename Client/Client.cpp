/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/23 19:02:32 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/*
** Client
*/

void Client::setup(int fd, Config *config)
{
	this->clientFd = fd;
	this->config = config;
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
	this->isCGI = false;
	this->responseString.clear();
	this->response.content.clear();
	this->response.contentType.clear();
	this->response.statusCode = 0;
	this->query.clear();
	this->server = NULL;
	this->location = NULL;
	this->cgi_state.inputPath.empty();
	this->cgi_state.outputPath.empty();
	this->cgi_state.pid = -1;
	this->cgi_state.running = false;
	this->generated = false;
	this->return_anyway = false;
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

/*
** CGI
*/

bool Client::isCGIRequest(const std::string &path)
{
	if (path.substr(path.find_last_of(".") + 1) == "py" || path.substr(path.find_last_of(".") + 1) == "php")
		return true;
	return false;
}

void Client::handleCGIRequest(const std::string &indexPath)
{
	if (!this->location)
		return this->setErrorResponse(500);

	std::string cgiPath;
	if (indexPath.find(".php") != std::string::npos)
		cgiPath = this->location->php_cgi_path;
	else
		cgiPath = this->location->python_cgi_path;

	this->cgi_state.outputPath = "/tmp/cgi_out_" + std::to_string(this->clientFd);
	this->cgi_state.inputPath = "/tmp/cgi_input_" + std::to_string(this->clientFd);
	this->cgi_state.running = true;

	pid_t pid = fork();
	if (pid < 0)
		return this->setErrorResponse(500);

	if (pid == 0)
	{
		if (this->location)
			alarm(this->location->cgi_timeout);

		if (this->method == METHOD_POST)
		{
			std::ifstream inputFile(this->cgi_state.inputPath);
			if (inputFile)
			{
				std::stringstream inputBuffer;
				inputBuffer << inputFile.rdbuf();
				this->body = inputBuffer.str();
				inputFile.close();
			}
			else
				exit(EXIT_FAILURE);
		}

		if (this->server->limit_client_body_size && this->body.size() > this->server->limit_client_body_size)
			exit(CONTENT_TO_LARGE);

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
		env["QUERY_STRING"] = this->query;
		env["SERVER_PROTOCOL"] = "HTTP/1.1";
		env["SERVER_SOFTWARE"] = "Webserv/1.0";
		env["SERVER_NAME"] = this->server_name;
		env["REQUEST_URI"] = this->path;
		env["PATH_INFO"] = this->path;
		env["PATH_TRANSLATED"] = indexPath;
		env["REMOTE_USER"] = "Webserv";
		
		char **envp = new char *[env.size() + 1];
		int i = 0;
		for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
		{
			envp[i] = strdup((it->first + "=" + it->second).c_str());
			++i;
		}
		envp[i] = NULL;

		int outFd = open(this->cgi_state.outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (outFd < 0)
		{
			for (int j = 0; j < i; ++j)
				free(envp[j]);
			delete[] envp;
			exit(EXIT_FAILURE);
		}
		dup2(outFd, STDOUT_FILENO);
		close(outFd);

		if (this->method == METHOD_POST)
		{
			int inputFd = open(this->cgi_state.inputPath.c_str(), O_RDONLY);
			if (inputFd < 0)
			{
				for (int j = 0; j < i; ++j)
					free(envp[j]);
				delete[] envp;
				exit(EXIT_FAILURE);
			}
			dup2(inputFd, STDIN_FILENO);
			close(inputFd);
		}

		const char *argv[] = {cgiPath.c_str(), indexPath.c_str(), NULL};

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
	this->cgi_state.pid = pid;
}

bool Client::checkCGICompletion()
{
	if (!this->cgi_state.running)
		return true;

	int status;
	pid_t result = waitpid(this->cgi_state.pid, &status, WNOHANG);

	if (result == 0)
		return false;

	this->cgi_state.running = false;

	if (result == -1)
	{
		this->setErrorResponse(500);
		return true;
	}

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM)
		this->setErrorResponse(504);
	else if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
	{
		std::ifstream cgiOutput(this->cgi_state.outputPath.c_str());
		if (cgiOutput)
		{
			std::stringstream buffer;
			std::string line;

			buffer << cgiOutput.rdbuf();

			while (std::getline(buffer, line))
			{
				size_t rpos = line.find('\r');
				if (rpos != std::string::npos)
					line = line.substr(0, rpos);
				if (line.find("Content-Type: ") != std::string::npos)
					this->response.contentType = line.substr(line.find("Content-Type: ") + 14);
			}

			size_t double_crlf = buffer.str().find("\r\n\r\n");
			if (double_crlf != std::string::npos)
				this->response.content = buffer.str().substr(double_crlf + 4);
			else
				this->response.content = buffer.str();

			this->response.statusCode = 200;
			cgiOutput.close();
		}
		else
			this->setErrorResponse(500);
	}
	else if (WIFEXITED(status) && WEXITSTATUS(status) == CONTENT_TO_LARGE)
		this->setErrorResponse(413);
	else
		this->setErrorResponse(500);

	unlink(this->cgi_state.outputPath.c_str());
	unlink(this->cgi_state.inputPath.c_str());

	return true;
}

/*
** Response
*/

void Client::setFinalResponse()
{
	if (this->location && !this->location->redirect.empty())
	{
		this->logRequest(response.statusCode);
		return;
	}

	if (this->response.contentType.empty())
	{
		if (this->response.statusCode >= 200 && this->response.statusCode < 300)
			this->response.contentType = getMimeType(this->path);
		else
			this->response.contentType = "text/html";
	}

	this->responseString = getHttpHeaders() + this->response.content;

	this->logRequest(response.statusCode);
}

void Client::setErrorResponse(int statusCode)
{
	this->response.content = loadErrorPage(getErrorPagePath(statusCode), statusCode);
	this->response.statusCode = statusCode;
}

void Client::setSuccessResponse(int statusCode, const std::string &path)
{
	if (this->isDirectory(path))
	{
		this->response.content = this->loadFiles(path);
		this->response.statusCode = statusCode;
		return;
	}
	this->response.content = this->loadFile(path);
	this->response.statusCode = statusCode;
}

void Client::checkConfigs()
{
	if (!this->server)
		return this->setErrorResponse(404);

	if (this->location)
	{
		if (std::find(this->location->accepted_methods.begin(), this->location->accepted_methods.end(), this->method) == this->location->accepted_methods.end())
			return this->setErrorResponse(405);

		if (!this->location->index.empty())
		{
			std::string indexPath = this->location->root_folder + "/" + this->location->index;

			if (!this->fileExists(indexPath))
				return this->setErrorResponse(404);

			if (!this->hasReadPermission(indexPath))
				return this->setErrorResponse(403);

			if (this->isCGIRequest(indexPath))
				return this->handleCGIRequest(indexPath);

			return this->setSuccessResponse(200, indexPath);
		}

		if (this->location->autoindex)
		{
			std::string defaultFilePath = this->location->root_folder + "/" + this->location->index;

			if (this->location->index.empty())
			{
				std::string fullPath = this->location->root_folder + this->path;
				if (!this->isDirectory(fullPath))
					return this->setErrorResponse(404);

				return this->setSuccessResponse(200, fullPath);
			}

			return this->setSuccessResponse(200, defaultFilePath);
		}

		if (this->location->index.empty())
			return this->setErrorResponse(404);
	}

	if (this->isDirectory(this->server->root_folder + this->path))
		return this->setSuccessResponse(200, this->server->root_folder + this->path);

	if (this->fileExists(this->server->root_folder + this->path))
	{
		// if (isCGIRequest(this->server->root_folder + this->path))
		// 	return handleCGIRequest(this->server->root_folder + this->path);
		return this->setSuccessResponse(200, this->server->root_folder + this->path);
	}

	this->setErrorResponse(404);
}

std::string Client::getHttpHeaders()
{
	int statusCode = this->response.statusCode;
	std::string headers;
	headers.clear();
	headers += "HTTP/1.1 " + std::to_string(statusCode) + " " + this->config->statusCodes[statusCode] + "\r\n";
	headers += "Content-Type: " + this->response.contentType + "\r\n";
	headers += "Content-Length: " + std::to_string(this->response.content.size()) + "\r\n";
	headers += "Connection: close\r\n";
	headers += "\r\n";
	return headers;
}

void Client::generateResponse()
{
	if (this->generated || this->return_anyway)
		return;
	this->generated = true;

	if (this->server && this->server->limit_client_body_size && this->content_length > this->server->limit_client_body_size)
	{
		this->response.statusCode = 413;
		this->response.content = this->loadErrorPage(this->getErrorPagePath(413), 413);
		return;
	}

	if ((!this->isChunked && !this->isContentLenght && this->method == METHOD_POST) || (!this->content_length && this->method == METHOD_POST))
	{
		this->response.statusCode = 400;
		this->response.content = this->loadErrorPage(this->getErrorPagePath(400), 400);
		return;
	}

	this->response.content.empty();
	this->response.contentType.empty();
	this->response.statusCode = 200;

	if (this->location && !this->location->redirect.empty())
	{
		this->response.statusCode = location->redirect_status_code;
		this->response.contentType = "text/html";

		this->responseString = "HTTP/1.1 " + std::to_string(response.statusCode) + " " + this->config->statusCodes[response.statusCode] + "\r\n";
		this->responseString += "Location: " + this->location->redirect + "\r\n";
		this->responseString += "Connection: close\r\n";
		this->responseString += "\r\n";
		return;
	}

	this->checkConfigs();
}

/*
** Configs
*/

Server *Client::getServer()
{
	for (std::vector<Server>::iterator serverIt = this->config->servers.begin(); serverIt != this->config->servers.end(); ++serverIt)
		if (std::find(serverIt->ports.begin(), serverIt->ports.end(), this->port) != serverIt->ports.end())
			if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->server_name) != serverIt->server_names.end())
				return &(*serverIt);
	return NULL;
}

Location *Client::getLocation()
{
	if (!this->server)
		return NULL;

	for (std::map<std::string, Location>::iterator locIt = this->server->locations.begin(); locIt != this->server->locations.end(); ++locIt)
		if (locIt->first == this->path)
			return &locIt->second;
	return NULL;
}

/*
** Loaders
*/

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

	if (!this->server)
		return error_page_path;

	std::map<int, std::string>::iterator it = this->server->error_pages.find(errorCode);
	if (it != this->server->error_pages.end())
		error_page_path = this->server->root_folder + "/" + it->second;
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
		"<tr><th>Name</th><th>Type</th></tr>"; // Add column for file path

	while ((entry = readdir(dir)) != NULL)
	{
		if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
			continue;

		std::string entryPath = directory + "/" + entry->d_name;
		std::string currentDirectory = directory.substr(this->server->root_folder.size(), directory.size());

		if (stat(entryPath.c_str(), &entryStat) != 0)
			continue;

		std::string fileType = S_ISDIR(entryStat.st_mode) ? "Directory" : "File";

		// Add the file path as a clickable link in the table
		fileListHTML += "<tr>";
		fileListHTML += "<td><a href='";
		fileListHTML += currentDirectory + (currentDirectory.back() == '/' ? "" : "/");
		fileListHTML += entry->d_name;
		fileListHTML += "'>" + std::string(entry->d_name) + "</a></td>";
		fileListHTML += "<td>" + fileType + "</td>";
		fileListHTML += "</tr>";
	}

	fileListHTML += "</table></body></html>";
	closedir(dir);
	return fileListHTML;
}

/*
** Checkers
*/

bool Client::fileExists(const std::string &path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISREG(statbuf.st_mode);
}

bool Client::isDirectory(const std::string &path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

bool Client::hasReadPermission(const std::string &path)
{
	return (access(path.c_str(), R_OK) == 0);
}

/*
** Parsing
*/

std::string Client::urlDecode(const std::string &str)
{
	std::string result;
	char ch;
	int hexValue;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == '%' && i + 2 < str.length() &&
			std::isxdigit(str[i + 1]) && std::isxdigit(str[i + 2]))
		{
			std::istringstream iss(str.substr(i + 1, 2));
			iss >> std::hex >> hexValue;
			ch = static_cast<char>(hexValue);
			result += ch;
			i += 2;
		}
		else if (str[i] == '+')
			result += ' ';
		else
			result += str[i];
	}
	return result;
}

void Client::handleFirstLine(std::istringstream &requestStream)
{
	std::string line;
	std::getline(requestStream, line);

	size_t rpos = line.find('\r');
	if (rpos != std::string::npos)
		line = line.substr(0, rpos);

	std::vector<std::string> parts;
	std::istringstream stream(line);

	std::string part;
	while (std::getline(stream, part, ' '))
		parts.push_back(part);

	if (parts.size() != 3)
	{
		this->response.statusCode = 400;
		this->response.content = this->loadErrorPage(this->getErrorPagePath(400), 400);
		this->return_anyway = true;
		return;
	}

	this->method = parts[0];
	if (this->method != METHOD_GET && this->method != METHOD_POST && this->method != METHOD_DELETE)
	{
		this->response.statusCode = 501;
		this->response.content = this->loadErrorPage(this->getErrorPagePath(501), 501);
		this->return_anyway = true;
		return;
	}

	std::string path = urlDecode(parts[1]);
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos)
	{
		this->path = path.substr(0, queryPos);
		this->query = path.substr(queryPos + 1);
	}
	else
		this->path = path;
	if (this->path != "/" && this->path.back() == '/')
		while (this->path.size() > 1 && this->path.back() == '/')
			this->path.pop_back();

	if (parts[2] != "HTTP/1.1")
	{
		this->response.statusCode = 505;
		this->response.content = this->loadErrorPage(this->getErrorPagePath(505), 505);
		this->return_anyway = true;
		return;
	}
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
				{
					this->response.statusCode = 400;
					this->response.content = this->loadErrorPage(this->getErrorPagePath(400), 400);
					this->return_anyway = true;
					return;
				}
				this->server_name = line.substr(hostPrefixPos + std::string(HOST_PREFIX).length(),
												colonPos - (hostPrefixPos + std::string(HOST_PREFIX).length()));
				this->port = std::atof(line.substr(colonPos + 1).c_str());
			}
			else if (transferEncoding != std::string::npos)
				this->isChunked = true;
			else if (colonPos != std::string::npos)
				this->headers[line.substr(0, colonPos)] = line.substr(colonPos + 2);
		}
	}

	this->server = this->getServer();
	this->location = this->getLocation();

	if (this->location)
	{
		this->upload_dir = this->location->upload_dir;
		if (this->isCGIRequest(this->location->index))
			this->isCGI = true;
	}
	// else if (this->isCGIRequest(this->path))
	// 	this->isCGI = true;
}

/*
** Getters
*/

const std::string &Client::getResponse()
{
	this->setFinalResponse();
	return this->responseString;
}

const std::string &Client::getBody() const { return this->body; }

const std::string &Client::getBoundary() const { return this->boundary; }

const std::map<std::string, std::string> &Client::getHeaders() const { return this->headers; }

const size_t &Client::getContentLength() const { return this->content_length; }

const std::string &Client::getMethod() const { return this->method; }

const bool &Client::getIsChunked() const { return this->isChunked; }

const bool &Client::getIsBinary() const { return this->isBinary; }

const bool &Client::getIsContentLenght() const { return this->isContentLenght; }

const bool &Client::getIsCGI() const { return this->isCGI; }

const std::string &Client::getUploadDir() const { return this->upload_dir; }

const std::string &Client::getContentType() const { return this->content_type; }