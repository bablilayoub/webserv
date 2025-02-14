/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/02/14 01:21:37 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/*
** Client
*/

Client::Client()
{
	this->parsed = false;
}

Client::Client(const Client &obj)
{
	*this = obj;
	this->clear();
}

Client &Client::operator=(const Client &obj)
{
	(void)obj;
	this->clear();
	return *this;
}

Client::~Client()
{
	if (this->response.file.is_open())
		this->response.file.close();
}

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
	this->connection.clear();
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
	this->cgi_response_headers.clear();
	this->response.headers.clear();
	this->response.headers_sent = false;
	this->response.lastReadPos = 0;
	this->response.done = false;
	this->response.sentSize = 0;
	this->response.filePath.clear();
	this->response.oldlastReadPos = 0;
}

void Client::logRequest(int statusCode)
{
	// Get the current time
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

	char timeBuffer[20];
	std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

	// Log the request details
	std::cout << "[" << timeBuffer << "] "
			  << BOLD << "Server: " << BLUE << std::setw(25) << std::left << (this->server_name + ":" + std::to_string(this->port)) << RESET
			  << BOLD << "Method: " << BLUE << std::setw(10) << std::left << this->method << RESET
			  << BOLD << "Path: " << WHITE << std::setw(30) << std::left << this->path << RESET
			  << BOLD << "Status: " << statusColor << std::setw(4) << std::left << statusCode << RESET
			  << std::endl;
}

/*
** CGI
*/

bool Client::isCGIRequest()
{
	if (!this->location)
		return false;

	if (this->location->php_cgi_path.empty() && this->sub_path.find(".php") != std::string::npos)
		return false;

	if (this->location->python_cgi_path.empty() && this->sub_path.find(".py") != std::string::npos)
		return false;

	std::string extension = this->sub_path.substr(this->sub_path.find_last_of('.') + 1);

	if (std::find(this->location->cgi_extensions.begin(), this->location->cgi_extensions.end(), extension) != this->location->cgi_extensions.end())
		return true;

	if (this->isDirectory(this->location->root_folder + this->sub_path))
	{
		if (!this->location->index.empty())
		{
			std::string extension = this->location->index.substr(this->location->index.find_last_of('.') + 1);
			if (std::find(this->location->cgi_extensions.begin(), this->location->cgi_extensions.end(), extension) != this->location->cgi_extensions.end())
			{
				if (this->location->index.find(".php") != std::string::npos && !this->location->php_cgi_path.empty())
					return true;
				if (this->location->index.find(".py") != std::string::npos && !this->location->python_cgi_path.empty())
					return true;
			}
		}
	}

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

	if (cgiPath.empty())
		return this->setErrorResponse(500);

	this->cgi_state.outputPath = "/tmp/cgi_out_" + std::to_string(this->clientFd);
	this->cgi_state.inputPath = "/tmp/cgi_input_" + std::to_string(this->clientFd);

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
		env["REMOTE_HOST"] = this->server_name;
		env["SERVER_PORT"] = std::to_string(port);
		env["REMOTE_IDENT"] = "Webserv";
		env["HTTP_USER_AGENT"] = "Client ID:" + std::to_string(clientFd);
		env["QUERY_STRING"] = this->query;
		env["SERVER_PROTOCOL"] = "HTTP/1.1";
		env["SERVER_SOFTWARE"] = "Webserv/1.0";
		env["SERVER_NAME"] = this->server_name;
		env["REQUEST_URI"] = this->path;
		env["PATH_INFO"] = this->path_info;
		env["PATH_TRANSLATED"] = this->location->root_folder + this->path_info;
		env["REMOTE_USER"] = "Webserv";

		for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it)
		{
			std::string key = "HTTP_" + it->first;
			for (size_t i = 0; i < key.size(); ++i)
				key[i] = std::toupper(key[i]);
			env[key] = it->second;
		}

		char **envp = new char *[env.size() + 1];
		int i = 0;
		for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
		{
			envp[i] = strdup((it->first + "=" + it->second).c_str());
			if (envp[i] == NULL)
				exit(EXIT_FAILURE);
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
		if (dup2(outFd, STDOUT_FILENO) < 0)
		{
			close(outFd);
			for (int j = 0; j < i; ++j)
				free(envp[j]);
			delete[] envp;
			exit(EXIT_FAILURE);
		}
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
			if (dup2(inputFd, STDIN_FILENO) < 0)
			{
				close(inputFd);
				for (int j = 0; j < i; ++j)
					free(envp[j]);
				delete[] envp;
				exit(EXIT_FAILURE);
			}
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
	this->cgi_state.running = true;
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
				if (line.empty() || line.find(":") == std::string::npos)
					break;
				size_t rpos = line.find('\r');
				if (rpos != std::string::npos)
					line = line.substr(0, rpos);

				size_t cpos = line.find(':');

				if (cpos == std::string::npos)
					break;

				if (cpos + 1 >= line.size())
					continue;

				std::string key = line.substr(0, cpos);
				std::string value = line.substr(cpos + 1);

				this->cgi_response_headers[key] = value;
			}

			size_t double_crlf = buffer.str().find("\r\n\r\n");
			if (double_crlf != std::string::npos)
				this->response.content = buffer.str().substr(double_crlf + 4);
			else
				this->response.content = buffer.str();

			this->response.totalSize = this->response.content.size();

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

	if (this->isCGI)
	{
		if (this->cgi_response_headers.count("Content-Type"))
			this->response.contentType = this->cgi_response_headers["Content-Type"];
		else
			this->response.contentType = "text/html";
	}

	if (this->response.contentType.empty())
	{
		if (this->response.statusCode >= 200 && this->response.statusCode < 300)
			this->response.contentType = getMimeType(this->path);
		else
			this->response.contentType = "text/html";
	}

	if (this->isCGI && this->cgi_response_headers.count("Location"))
	{
		this->response.headers = "HTTP/1.1 302 Found\r\n";
		this->response.headers += "Location: " + this->cgi_response_headers["Location"] + "\r\n";
		this->response.headers += "Connection: close\r\n";
		this->response.headers += "\r\n";
		this->logRequest(302);
		return;
	}

	this->response.headers = getHttpHeaders();

	this->logRequest(response.statusCode);
}

void Client::setErrorResponse(int statusCode)
{
	if (this->method.empty())
		this->method = "UNKNOWN";
	if (this->path.empty())
		this->path = "UNKNOWN";
	if (this->server_name.empty())
		this->server_name = "UNKNOWN";

	this->response.content = loadErrorPage(getErrorPagePath(statusCode), statusCode);
	this->response.statusCode = statusCode;
	this->return_anyway = true;
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
	this->response.contentType = getMimeType(path);
}

void Client::startProcessing()
{
	if (this->location)
	{
		if (this->isDirectory(this->location->root_folder + this->sub_path))
		{
			if (!this->location->index.empty())
			{
				std::string indexPath;
				if (this->sub_path.back() == '/')
					indexPath = this->location->root_folder + this->sub_path + this->location->index;
				else
					indexPath = this->location->root_folder + this->sub_path + "/" + this->location->index;
				if (!this->fileExists(indexPath))
				{
					if (this->location->autoindex)
					{
						std::string fullPath = this->location->root_folder + this->sub_path;
						if (!this->isDirectory(fullPath))
							return this->setErrorResponse(404);

						return this->setSuccessResponse(200, fullPath);
					}
					return this->setErrorResponse(404);
				}

				if (!this->hasReadPermission(indexPath))
					return this->setErrorResponse(403);

				if (this->isCGI)
					return this->handleCGIRequest(indexPath);

				return this->setSuccessResponse(200, indexPath);
			}

			if (this->location->autoindex)
			{
				std::string defaultFilePath = this->location->root_folder + "/" + this->location->index;

				if (this->location->index.empty())
				{
					std::string fullPath = this->location->root_folder + this->sub_path;
					if (!this->isDirectory(fullPath))
						return this->setErrorResponse(404);

					return this->setSuccessResponse(200, fullPath);
				}

				return this->setSuccessResponse(200, defaultFilePath);
			}

			if (this->location->index.empty())
			{
				std::string defaultFilePath = this->location->root_folder + this->sub_path + "/index.html";

				if (!this->fileExists(defaultFilePath))
					return this->setErrorResponse(404);

				if (!this->hasReadPermission(defaultFilePath))
					return this->setErrorResponse(403);

				return this->setSuccessResponse(200, defaultFilePath);
			}
		}

		if (this->fileExists(this->location->root_folder + this->sub_path))
		{
			if (!this->hasReadPermission(this->location->root_folder + this->sub_path))
				return this->setErrorResponse(403);

			if (this->isCGI)
				return this->handleCGIRequest(this->location->root_folder + this->sub_path);

			return this->setSuccessResponse(200, this->location->root_folder + this->sub_path);
		}
	}

	if (this->isDirectory(this->server->root_folder + this->sub_path))
		return this->setSuccessResponse(200, this->server->root_folder + this->sub_path);

	if (this->fileExists(this->server->root_folder + this->sub_path))
		return this->setSuccessResponse(200, this->server->root_folder + this->sub_path);

	this->setErrorResponse(404);
}

std::string Client::getHttpHeaders()
{
	int statusCode = this->response.statusCode;
	std::string headers;
	headers.clear();
	headers += "HTTP/1.1 " + std::to_string(statusCode) + " " + this->config->statusCodes[statusCode] + "\r\n";
	headers += "Content-Type: " + this->response.contentType + "\r\n";
	headers += "Content-Length: " + std::to_string(this->response.totalSize) + "\r\n";
	if (this->isCGI)
	{
		for (std::map<std::string, std::string>::iterator it = this->cgi_response_headers.begin(); it != this->cgi_response_headers.end(); ++it)
		{
			if (it->first == "Content-Type" || it->first == "Content-Length")
				continue;
			headers += it->first + ": " + it->second + "\r\n";
		}
	}
	if (this->connection.empty())
		this->connection = "close";
	headers += "Connection: " + this->connection + "\r\n";
	headers += "Accept-Ranges: none\r\n";
	headers += "\r\n";
	return headers;
}

void Client::generateResponse()
{
	if (this->generated || this->return_anyway)
		return;
	this->generated = true;

	this->response.content.empty();
	this->response.contentType.empty();
	this->response.statusCode = 200;

	if (this->location && !this->location->redirect.empty())
	{
		this->response.statusCode = location->redirect_status_code;
		this->response.contentType = "text/html";

		this->response.headers = "HTTP/1.1 " + std::to_string(response.statusCode) + " " + this->config->statusCodes[response.statusCode] + "\r\n";
		this->response.headers += "Location: " + this->location->redirect + "\r\n";
		this->response.headers += "Connection: close\r\n";
		this->response.headers += "\r\n";
		return;
	}

	this->startProcessing();
}

/*
** Configs
*/

Server *Client::getServer()
{
	Server *defaultServer = NULL;

	for (std::vector<Server>::iterator serverIt = this->config->servers.begin(); serverIt != this->config->servers.end(); ++serverIt)
	{
		if (std::find(serverIt->ports.begin(), serverIt->ports.end(), this->port) != serverIt->ports.end())
		{
			if (!defaultServer)
				defaultServer = &(*serverIt);

			if (std::find(serverIt->server_names.begin(), serverIt->server_names.end(), this->server_name) != serverIt->server_names.end())
				return &(*serverIt);
		}
	}

	return defaultServer;
}

Location *Client::getLocation()
{
	if (!this->server)
		return NULL;

	Location *closestMatch = NULL;
	size_t longestMatchLength = 0;

	for (std::map<std::string, Location>::iterator locIt = this->server->locations.begin();
		 locIt != this->server->locations.end(); ++locIt)
	{

		const std::string &locationPath = locIt->first;

		if (this->path.compare(0, locationPath.size(), locationPath) == 0)
		{
			if (this->path.size() > locationPath.size() && this->path[locationPath.size()] != '/')
				continue;

			if (locationPath.size() > longestMatchLength)
			{
				longestMatchLength = locationPath.size();
				closestMatch = &locIt->second;
			}
		}
	}

	if (!closestMatch)
	{
		std::string tempPath = this->path;

		size_t pos = tempPath.find(".php");
		if (pos != std::string::npos)
			tempPath = tempPath.substr(0, pos + 4);
		pos = tempPath.find(".py");
		if (pos != std::string::npos)
			tempPath = tempPath.substr(0, pos + 3);

		std::map<std::string, Location>::iterator rootLocation = this->server->locations.find("/");
		if (rootLocation != this->server->locations.end() && tempPath.find('/') == 0 && tempPath.find('/', 1) == std::string::npos)
			closestMatch = &rootLocation->second;
	}

	if (closestMatch)
	{
		this->sub_path = this->path.substr(longestMatchLength);
		if (this->sub_path.empty())
			this->sub_path = "/";
		else if (this->sub_path[0] != '/')
			this->sub_path = "/" + this->sub_path;

		for (std::vector<std::string>::iterator it = closestMatch->cgi_extensions.begin();
			 it != closestMatch->cgi_extensions.end(); ++it)
		{
			size_t pos = this->sub_path.find(*it);
			if (pos != std::string::npos)
			{
				if (this->sub_path.size() > pos + it->size() && this->sub_path[pos + it->size()] != '/')
					continue;

				this->path_info = this->sub_path.substr(pos + it->size());
				this->sub_path = this->sub_path.substr(0, pos + it->size());
				break;
			}
		}
	}
	else
	{
		this->sub_path = this->path;
		this->path_info = this->path;
	}

	return closestMatch;
}

/*
** Loaders
*/

std::string Client::loadErrorPage(const std::string &filePath, int statusCode)
{
	std::stringstream buffer;
	std::string statusMessage = this->config->statusCodes.count(statusCode) > 0
									? this->config->statusCodes.at(statusCode)
									: "Unknown Error";

	// Determine background and accent colors based on status code
	std::string bgGradient, accentColor, borderColor;
	if (statusCode >= 500)
	{
		bgGradient = "bg-gradient-to-br from-red-900 via-red-800 to-red-900";
		accentColor = "red";
		borderColor = "border-red-300";
	}
	else if (statusCode >= 400)
	{
		bgGradient = "bg-gradient-to-br from-orange-900 via-orange-800 to-orange-900";
		accentColor = "orange";
		borderColor = "border-orange-300";
	}
	else if (statusCode >= 300)
	{
		bgGradient = "bg-gradient-to-br from-purple-900 via-purple-800 to-purple-900";
		accentColor = "purple";
		borderColor = "border-purple-300";
	}
	else
	{
		bgGradient = "bg-gradient-to-br from-blue-900 via-blue-800 to-blue-900";
		accentColor = "blue";
		borderColor = "border-blue-300";
	}

	std::string repeatedDivs;
	for (int i = 0; i < 9; ++i)
		repeatedDivs += "<div class=\"aspect-square rounded-lg bg-white\"></div>";

	std::string html =
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"    <meta charset=\"UTF-8\">\n"
		"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"    <title>Error " +
		std::to_string(statusCode) + "</title>\n"
									 "    <script src=\"https://cdn.tailwindcss.com\"></script>\n"
									 "</head>\n"
									 "<body class=\"min-h-screen " +
		bgGradient + " flex items-center justify-center p-4 sm:p-8\">\n"
					 "    <div class=\"relative w-full max-w-2xl\">\n"
					 "        <!-- Decorative circles -->\n"
					 "        <div class=\"hidden lg:block absolute -top-20 -left-20 w-40 h-40 bg-" +
		accentColor + "-500 rounded-full opacity-10 animate-pulse\"></div>\n"
					  "        <div class=\"hidden lg:block absolute -bottom-20 -right-20 w-40 h-40 bg-" +
		accentColor + "-500 rounded-full opacity-10 animate-pulse delay-75\"></div>\n"
					  "        \n"
					  "        <!-- Main content -->\n"
					  "        <div class=\"relative backdrop-blur-lg bg-white bg-opacity-10 rounded-2xl p-8 border " +
		borderColor + " border-opacity-20 shadow-2xl\">\n"
					  "            <div class=\"grid grid-cols-1 md:grid-cols-2 gap-8 items-center\">\n"
					  "                <!-- Error details -->\n"
					  "                <div class=\"text-white space-y-6\">\n"
					  "                    <h1 class=\"text-8xl font-bold tracking-tighter\">" +
		std::to_string(statusCode) + "</h1>\n"
									 "                    <h2 class=\"text-2xl font-medium leading-tight\">" +
		statusMessage + "</h2>\n"
						"                    <p class=\"text-" +
		accentColor + "-200\">\n"
					  "                        We're sorry, but we couldn't find the page you were looking for.\n"
					  "                    </p>\n"
					  "                    <a href=\"/\" class=\"inline-flex items-center px-6 py-3 rounded-lg bg-white \n"
					  "                              text-" +
		accentColor + "-900 hover:bg-opacity-90 transition-all \n"
					  "                              transform hover:scale-105 hover:shadow-lg\">\n"
					  "                        <svg class=\"w-5 h-5 mr-2\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
					  "                            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" \n"
					  "                                  d=\"M7 16l-4-4m0 0l4-4m-4 4h18\">\n"
					  "                            </path>\n"
					  "                        </svg>\n"
					  "                        Go Back Home\n"
					  "                    </a>\n"
					  "                </div>\n"
					  "                \n"
					  "                <!-- Decorative pattern -->\n"
					  "                <div class=\"hidden md:block\">\n"
					  "                    <div class=\"grid grid-cols-3 gap-4 opacity-20\">\n"
					  "                        " +
		repeatedDivs + "\n"
					   "                    </div>\n"
					   "                </div>\n"
					   "            </div>\n"
					   "        </div>\n"
					   "    </div>\n"
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

	this->response.totalSize = html.size();
	this->response.contentType = "text/html";

	return html;
}

std::string Client::getMimeType(const std::string &path)
{
	size_t dotPos = path.rfind('.');
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
	std::string content;

	if (!this->response.file.is_open())
	{
		this->response.file.open(filePath.c_str(), std::ios::binary | std::ios::in);
		if (!this->response.file.is_open())
		{
			std::cout << "Failed to open file: " << filePath << std::endl;
			return this->loadErrorPage(this->getErrorPagePath(404), 404);
		}

		this->response.filePath = filePath;

		this->response.file.seekg(0, std::ios::end);
		this->response.totalSize = this->response.file.tellg();
		this->response.file.seekg(0, std::ios::beg);
	}

	if (this->response.lastReadPos > 0)
		this->response.file.seekg(this->response.lastReadPos);

	if (this->response.totalSize > 0)
	{
		size_t leftToRead = this->response.totalSize - this->response.lastReadPos;
		size_t readSize = leftToRead > BYTES_TO_READ ? BYTES_TO_READ : leftToRead;

		char buffer[BYTES_TO_READ];
		this->response.file.read(buffer, readSize);
		content = std::string(buffer, this->response.file.gcount());
		this->response.oldlastReadPos = this->response.lastReadPos;
		this->response.lastReadPos = this->response.file.tellg();
	}

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
		"<meta charset='UTF-8'>"
		"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
		"<script src='https://cdn.tailwindcss.com'></script>"
		"<title>Directory Listing</title>"
		"<link rel='icon' type='image/png' href='./icon.png' />"
		"</head>"
		"<body class='bg-gray-50 min-h-screen'>"
		"<div class='min-h-screen'>"
		"<header class='bg-gradient-to-r from-blue-600 to-blue-700 shadow-lg'>"
		"<div class='max-w-7xl mx-auto py-6 px-4'>"
		"<h1 class='text-2xl font-bold text-white text-center'>Directory Listing</h1>"
		"</div>"
		"</header>"
		"<main class='max-w-7xl mx-auto py-6 px-4 sm:px-6 lg:px-8'>"
		"<div class='bg-white rounded-lg shadow-md overflow-hidden'>"
		"<div class='overflow-x-auto'>"
		"<table class='min-w-full divide-y divide-gray-200'>"
		"<thead class='bg-gray-50'>"
		"<tr>"
		"<th class='px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider'>Name</th>"
		"<th class='px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider'>Type</th>"
		"</tr>"
		"</thead>"
		"<tbody class='bg-white divide-y divide-gray-200'>";

	while ((entry = readdir(dir)) != NULL)
	{
		if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
			continue;

		std::string entryPath = directory + "/" + entry->d_name;
		std::string currentDirectory;

		if (this->location)
			currentDirectory = directory.substr(this->location->root_folder.size(), directory.size());
		else
			currentDirectory = directory.substr(this->server->root_folder.size(), directory.size());

		if (stat(entryPath.c_str(), &entryStat) != 0)
			continue;

		std::string fileType = S_ISDIR(entryStat.st_mode) ? "Directory" : "File";
		std::string typeClass = S_ISDIR(entryStat.st_mode)
									? "bg-blue-100 text-blue-800"
									: "bg-gray-100 text-gray-800";

		fileListHTML += "<tr class='hover:bg-gray-50 transition-colors duration-150 ease-in-out'>";
		fileListHTML += "<td class='px-6 py-4 whitespace-nowrap'><div class='flex items-center'>";

		// Add icon based on type
		if (S_ISDIR(entryStat.st_mode))
		{
			fileListHTML += "<svg class='flex-shrink-0 h-5 w-5 text-blue-500 mr-3' fill='none' stroke='currentColor' viewBox='0 0 24 24'>"
							"<path stroke-linecap='round' stroke-linejoin='round' stroke-width='2' d='M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z'/>"
							"</svg>";
		}
		else
		{
			fileListHTML += "<svg class='flex-shrink-0 h-5 w-5 text-gray-400 mr-3' fill='none' stroke='currentColor' viewBox='0 0 24 24'>"
							"<path stroke-linecap='round' stroke-linejoin='round' stroke-width='2' d='M7 21h10a2 2 0 002-2V9.414a1 1 0 00-.293-.707l-5.414-5.414A1 1 0 0012.586 3H7a2 2 0 00-2 2v14a2 2 0 002 2z'/>"
							"</svg>";
		}

		fileListHTML += "<a href='";
		fileListHTML += currentDirectory + (currentDirectory.back() == '/' ? "" : "/");
		fileListHTML += entry->d_name;
		fileListHTML += "' class='text-sm font-medium text-gray-900 hover:text-blue-600 transition-colors duration-150'>";
		fileListHTML += entry->d_name;
		fileListHTML += "</a></div></td>";

		fileListHTML += "<td class='px-6 py-4 whitespace-nowrap'>";
		fileListHTML += "<span class='inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium " + typeClass + "'>";
		fileListHTML += fileType;
		fileListHTML += "</span></td></tr>";
	}

	fileListHTML +=
		"</tbody>"
		"</table>"
		"</div>"
		"</div>"
		"</main>"
		"</div>"
		"</body>"
		"</html>";

	closedir(dir);

	this->response.contentType = "text/html";
	this->response.totalSize = fileListHTML.size();

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
	std::string firstLine;
	if (!std::getline(requestStream, firstLine))
	{
		setErrorResponse(400);
		return;
	}
	if (!firstLine.empty() && firstLine.back() == '\r')
		firstLine.pop_back();

	std::istringstream lineStream(firstLine);
	std::string method, rawPath, httpVersion, extra;
	if (!(lineStream >> method >> rawPath >> httpVersion) || (lineStream >> extra))
	{
		setErrorResponse(400);
		return;
	}

	if (method != METHOD_GET && method != METHOD_POST && method != METHOD_DELETE)
	{
		setErrorResponse(501);
		return;
	}
	this->method = method;

	std::string decodedPath = urlDecode(rawPath);
	size_t queryPos = decodedPath.find('?');
	if (queryPos != std::string::npos)
	{
		this->path = decodedPath.substr(0, queryPos);
		this->query = decodedPath.substr(queryPos + 1);
	}
	else
		this->path = decodedPath;

	size_t hashPos = this->path.find('#');
	if (hashPos != std::string::npos)
		this->path = this->path.substr(0, hashPos);

	if (this->path != "/" && !this->path.empty() && this->path.back() == '/')
		while (this->path.size() > 1 && this->path.back() == '/')
			this->path.pop_back();

	std::vector<std::string> pathParts;
	std::istringstream pathStream(this->path);
	std::string segment;
	while (std::getline(pathStream, segment, '/'))
	{
		if (segment == "..")
		{
			setErrorResponse(403);
			return;
		}
		else if (!segment.empty() && segment != ".")
			pathParts.push_back(segment);
	}

	this->path = "/";
	for (size_t i = 0; i < pathParts.size(); i++)
	{
		this->path += pathParts[i];
		if (i < pathParts.size() - 1)
			this->path += "/";
	}

	if (httpVersion != "HTTP/1.1")
	{
		setErrorResponse(505);
		return;
	}
}

std::string Client::trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\n\r");
	size_t end = s.find_last_not_of(" \t\n\r");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

void Client::parse(const std::string &request)
{
	size_t headerEnd = request.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		setErrorResponse(400);
		this->parsed = true;
		return;
	}

	this->clear();

	std::string headerSection = request.substr(0, headerEnd);
	std::istringstream headerStream(headerSection);

	this->handleFirstLine(headerStream);
	if (this->return_anyway)
	{
		this->parsed = true;
		return;
	}

	std::string line;
	while (std::getline(headerStream, line))
	{
		if (line.empty())
			continue;

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
		{
			setErrorResponse(400);
			this->parsed = true;
			return;
		}

		std::string key = this->trim(line.substr(0, colonPos));
		std::string value = this->trim(line.substr(colonPos + 1));

		std::string lowerKey = key;
		std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

		if (lowerKey == "content-length")
		{
			try
			{
				this->content_length = std::stoul(value);
				this->isContentLenght = true;
			}
			catch (const std::exception &e)
			{
				setErrorResponse(400);
				this->parsed = true;
				return;
			}
		}
		else if (lowerKey == "content-type")
		{
			this->content_type = value;
			size_t boundaryPos = this->content_type.find("boundary=");
			if (boundaryPos != std::string::npos)
				this->boundary = this->content_type.substr(boundaryPos + 9);
			else if (this->content_type.find("application/x-www-form-urlencoded") == std::string::npos)
				this->isBinary = true;
		}
		else if (lowerKey == "host")
		{
			size_t colonInHost = value.find(':');
			if (colonInHost == std::string::npos)
			{
				setErrorResponse(400);
				this->parsed = true;
				return;
			}
			this->server_name = value.substr(0, colonInHost);
			if (this->server_name.empty() || this->server_name.find_first_of(" \t\n\r") != std::string::npos)
			{
				setErrorResponse(400);
				this->parsed = true;
				return;
			}
			std::string portStr = value.substr(colonInHost + 1);
			if (portStr.find_first_not_of("0123456789") != std::string::npos)
			{
				setErrorResponse(400);
				this->parsed = true;
				return;
			}
			try
			{
				this->port = static_cast<unsigned short>(std::stoul(portStr));
			}
			catch (const std::exception &e)
			{
				setErrorResponse(400);
				this->parsed = true;
				return;
			}
		}
		else if (lowerKey == "transfer-encoding")
		{
			if (value.find("chunked") != std::string::npos)
				this->isChunked = true;
			else
			{
				setErrorResponse(501);
				this->parsed = true;
				return;
			}
		}
		else if (lowerKey == "connection")
			this->connection = value;
		else
			this->headers[key] = value;
	}

	if (this->server_name.empty() || this->port == 0 ||
		this->path.empty() || this->method.empty())
	{
		setErrorResponse(400);
		this->parsed = true;
		return;
	}

	this->server = this->getServer();

	if (!this->server)
	{
		setErrorResponse(503);
		this->parsed = true;
		return; 
	}

	this->location = this->getLocation();

	if (this->location && this->location->redirect.empty() && std::find(this->location->accepted_methods.begin(), this->location->accepted_methods.end(), this->method) == this->location->accepted_methods.end())
	{
		setErrorResponse(405);
		this->parsed = true;
		return;
	}

	if (!this->location && this->method != METHOD_GET)
	{
		setErrorResponse(405);
		this->parsed = true;
		return;
	}

	if (this->server && this->server->limit_client_body_size &&
		this->content_length > this->server->limit_client_body_size)
	{
		setErrorResponse(413);
		this->parsed = true;
		return;
	}

	if (!this->isChunked && !this->isContentLenght && this->method == METHOD_POST)
	{
		setErrorResponse(400);
		this->parsed = true;
		return;
	}
	if (this->method == METHOD_POST && this->content_length == 0)
	{
		setErrorResponse(400);
		this->parsed = true;
		return;
	}

	if (this->location)
	{
		this->upload_dir = this->location->upload_dir;
		if (this->isCGIRequest())
			this->isCGI = true;
	}
	if (this->isCGI && this->content_type.empty() && this->method == METHOD_POST)
	{
		setErrorResponse(400);
		this->parsed = true;
		return;
	}

	this->parsed = true;
}

void Client::sendResponse()
{
	if (this->response.done || (this->getIsCGI() && !this->checkCGICompletion()))
		return;

	std::string fullResponse;
	bool firstTime = false;

	if (!this->response.headers_sent)
	{
		firstTime = true;
		this->setFinalResponse();

		fullResponse = this->response.headers + this->response.content;

		if (this->location && !this->location->redirect.empty())
			this->response.done = true;
	}

	if (!firstTime)
	{
		if (this->response.filePath.empty())
		{
			this->response.done = true;
			return;
		}

		fullResponse = this->loadFile(this->response.filePath);
		if (fullResponse.empty())
		{
			this->response.done = true;
			return;
		}
	}

	ssize_t bytes_sent = send(this->clientFd, fullResponse.c_str(), fullResponse.size(), 0);
	if (bytes_sent == -1)
	{
		this->response.lastReadPos = this->response.oldlastReadPos;
		return;
	}
	if (bytes_sent == 0)
	{
		this->response.done = true;
		return;
	}

	if (firstTime)
		this->response.headers_sent = true;

	this->response.sentSize += bytes_sent;

	if ((this->response.totalSize + this->response.headers.size()) == this->response.sentSize)
		this->response.done = true;
}

/*
** Getters
*/

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