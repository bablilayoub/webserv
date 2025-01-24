/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:49:18 by abablil           #+#    #+#             */
/*   Updated: 2025/01/24 12:32:20 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::~Config()
{
	if (configFile)
		configFile.close();
}

std::string Config::trimTrailingSlash(const std::string &path)
{
	if (path.empty())
		return path;
	std::string trimmed = path;
	while (trimmed.back() == '/')
		trimmed.pop_back();
	return trimmed;
}

bool Config::isValidDirectory(const std::string &path)
{
	struct stat statbuf;

	if (stat(path.c_str(), &statbuf) != 0)
		return false;

	if (!S_ISDIR(statbuf.st_mode))
		return false;

	if (access(path.c_str(), R_OK | W_OK | X_OK) != 0)
		return false;

	return true;
}

bool Config::isCGI(const std::string &path, const std::string &cgi)
{
	if (path.substr(path.find_last_of(".") + 1) == cgi)
		return true;
	return false;
}

bool Config::isValidCGI(const std::string &path)
{
	struct stat statbuf;

	if (stat(path.c_str(), &statbuf) != 0)
		return false;

	if (!S_ISREG(statbuf.st_mode))
		return false;

	if (access(path.c_str(), R_OK | X_OK) != 0)
		return false;

	return true;
}

void Config::trimWhitespace(std::string &line)
{
	size_t start = line.find_first_not_of(" \t\r\n");
	size_t end = line.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos)
		line.clear();
	else
		line = line.substr(start, end - start + 1);
}

void Config::parseKeyValue(const std::string &line, std::string &key, std::string &value)
{
	size_t pos = line.find(' ');
	if (pos == std::string::npos)
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Missing space between key and value");

	key = line.substr(0, pos);
	value = line.substr(pos + 1);

	if (value.back() != ';')
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Missing semicolon at the end");

	value.pop_back();
	this->trimWhitespace(value);
}

void Config::processServerBlock()
{
	if (!blockStack.empty())
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Nested server blocks are not allowed");
	blockStack.push("server");
	currentServer = Server();
}

void Config::processLocationBlock(const std::string &line)
{
	if (blockStack.empty() || blockStack.top() != "server")
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Location block must be inside a server block");

	blockStack.push("location");
	size_t pos = line.find(' ');
	if (pos == std::string::npos || pos + 1 >= line.size() - 1)
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid location syntax");

	locationPath = line.substr(pos + 1, line.size() - pos - 2);
	this->trimWhitespace(locationPath);

	if (locationPath != "/")
		locationPath = trimTrailingSlash(locationPath);

	currentLocation = Location();
	currentLocation.autoindex = false;
}

void Config::processClosingBrace()
{
	if (blockStack.empty())
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Unexpected closing brace");

	std::string closingBlock = blockStack.top();
	blockStack.pop();

	if (closingBlock == "location")
	{
		if (locationPath.empty())
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Location path is not specified");
		if (currentLocation.root_folder.empty())
			currentLocation.root_folder = currentServer.root_folder;
		if (std::find(currentLocation.accepted_methods.begin(), currentLocation.accepted_methods.end(), METHOD_POST) != currentLocation.accepted_methods.end())
			if (currentLocation.upload_dir.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": upload_dir is required for the POST method.");

		if (!currentLocation.index.empty() && this->isCGI(currentLocation.index, "php"))
		{
			if (currentLocation.php_cgi_path.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": This location requires php_cgi_path.");
			if (currentLocation.cgi_timeout <= 0)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": This location requires cgi_timeout.");
		}

		if (!currentLocation.index.empty() && this->isCGI(currentLocation.index, "py"))
		{
			if (currentLocation.python_cgi_path.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": This location requires python_cgi_path.");
			if (currentLocation.cgi_timeout <= 0)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": This location requires cgi_timeout.");
		}

		currentServer.locations[locationPath] = currentLocation;
		locationPath.clear();
	}
	else if (closingBlock == "server")
	{
		if (currentServer.root_folder.empty())
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": root_folder is not specified");
		for (size_t i = 0; i < currentServer.ports.size(); i++)
			if (currentServer.ports[i] < 1 || currentServer.ports[i] > 65535)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid port number");

		servers.push_back(currentServer);
	}
}

int Config::parseInt(const std::string &value)
{
	for (size_t i = 0; i < value.size(); i++)
		if (!std::isdigit(value[i]))
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid number");
	return std::atof(value.c_str());
}

bool Config::isValidIpv4(const std::string &ip)
{
	std::vector<std::string> parts;
	std::istringstream stream(ip);
	std::string part;

	while (std::getline(stream, part, '.'))
	{
		if (part.empty())
			return false;
		for (size_t i = 0; i < part.size(); i++)
			if (!std::isdigit(part[i]))
				return false;
		parts.push_back(part);
	}

	if (parts.size() != 4)
		return false;

	for (size_t i = 0; i < parts.size(); i++)
	{
		int num = std::atof(parts[i].c_str());
		if (num < 0 || num > 255)
			return false;
	}

	return true;
}

void Config::handleKeyValue(const std::string &line)
{
	std::string key, value;
	this->parseKeyValue(line, key, value);

	if (blockStack.top() == "server")
	{
		if (key == "host")
		{
			if (value == "localhost")
				currentServer.host = "127.0.0.1";
			else if (!this->isValidIpv4(value))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid host");
			else
				currentServer.host = value;
		}
		else if (key == "listen")
		{
			currentServer.ports.clear();
			std::istringstream stream(value);
			std::string port;

			while (std::getline(stream, port, ' '))
			{
				this->trimWhitespace(port);
				currentServer.ports.push_back(this->parseInt(port));
			}

			if (currentServer.ports.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": listen cannot be empty");
		}
		else if (key == "server_names")
		{
			currentServer.server_names.clear();
			std::istringstream stream(value);
			std::string server_name;

			while (std::getline(stream, server_name, ' '))
			{
				this->trimWhitespace(server_name);

				for (size_t i = 0; i < server_name.size(); i++)
					if (!std::isalnum(server_name[i]) && server_name[i] != '.' && server_name[i] != '-')
						throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid server_name '" + server_name + "'");

				currentServer.server_names.push_back(server_name);
			}
			if (currentServer.server_names.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": server_names cannot be empty");
		}
		else if (key == "limit_client_body_size")
			currentServer.limit_client_body_size = this->parseInt(value);
		else if (key == "root_folder")
		{
			currentServer.root_folder = trimTrailingSlash(value);
			if (!isValidDirectory(currentServer.root_folder))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid root_folder directory or permissions.");
		}
		else if (key == "error_page")
		{
			size_t pos = value.find(' ');
			if (pos == std::string::npos)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid error_page value");

			int errorCode = this->parseInt(value.substr(0, pos));
			std::string errorPage = value.substr(pos + 1);
			currentServer.error_pages[errorCode] = errorPage;
		}
		else
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Unknown server key: " + key);
	}
	else if (blockStack.top() == "location")
	{
		if (key == "upload_dir")
		{
			currentLocation.upload_dir = trimTrailingSlash(value);
			if (!isValidDirectory(currentLocation.upload_dir))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid upload_dir directory or permissions.");
		}
		else if (key == "redirect")
		{
			size_t pos = value.find(' ');
			if (pos == std::string::npos)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid redirect value, must be in the format 'status_code url'");

			currentLocation.redirect_status_code = this->parseInt(value.substr(0, pos));
			if (currentLocation.redirect_status_code < 300 || currentLocation.redirect_status_code > 308)
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid redirect status code, must be between 300 and 308");
			currentLocation.redirect = value.substr(pos + 1);
		}
		else if (key == "cgi_timeout")
			currentLocation.cgi_timeout = this->parseInt(value);
		else if (key == "root_folder")
		{
			currentLocation.root_folder = trimTrailingSlash(value);
			if (!isValidDirectory(currentLocation.root_folder))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid root_folder directory or permissions.");
		}
		else if (key == "index")
			currentLocation.index = value;
		else if (key == "autoindex")
		{
			if (value == "on")
				currentLocation.autoindex = true;
			else if (value == "off")
				currentLocation.autoindex = false;
			else
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": autoindex must be on/off");
		}
		else if (key == "php_cgi_path")
		{
			currentLocation.php_cgi_path = trimTrailingSlash(value);
			if (!this->isValidCGI(currentLocation.php_cgi_path))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid php_cgi_path executable or permissions.");
		}
		else if (key == "python_cgi_path")
		{
			currentLocation.python_cgi_path = trimTrailingSlash(value);
			if (!this->isValidCGI(currentLocation.python_cgi_path))
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid python_cgi_path executable or permissions.");
		}
		else if (key == "accepted_methods")
		{
			currentLocation.accepted_methods.clear();
			std::istringstream stream(value);
			std::string method;

			while (std::getline(stream, method, ','))
			{
				this->trimWhitespace(method);
				if (method != METHOD_GET && method != METHOD_POST && method != METHOD_DELETE)
					throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid method (GET, POST, DELETE only)");
				currentLocation.accepted_methods.push_back(method);
			}

			if (currentLocation.accepted_methods.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": accepted_methods cannot be empty");
		}
		else if (key == "cgi_extensions")
		{
			currentLocation.cgi_extensions.clear();
			std::istringstream stream(value);
			std::string extension;

			std::string validExtensions[] = {"py", "php"};

			while (std::getline(stream, extension, ','))
			{
				this->trimWhitespace(extension);
				if (std::find(std::begin(validExtensions), std::end(validExtensions), extension) == std::end(validExtensions))
					throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid CGI extension (py, php only)");
				currentLocation.cgi_extensions.push_back(extension);
			}

			if (currentLocation.cgi_extensions.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": cgi_extensions cannot be empty");
		}
		else
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Unknown location key: " + key);
	}
	else
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid syntax");
}

void Config::initStatusCodes()
{
	this->statusCodes[200] = "OK";
	this->statusCodes[201] = "Created";
	this->statusCodes[202] = "Accepted";
	this->statusCodes[204] = "No Content";
	this->statusCodes[300] = "Multiple Choices";
	this->statusCodes[301] = "Moved Permanently";
	this->statusCodes[302] = "Found";
	this->statusCodes[303] = "See Other";
	this->statusCodes[304] = "Not Modified";
	this->statusCodes[307] = "Temporary Redirect";
	this->statusCodes[308] = "Permanent Redirect";
	this->statusCodes[400] = "Bad Request";
	this->statusCodes[401] = "Unauthorized";
	this->statusCodes[403] = "Forbidden";
	this->statusCodes[404] = "Not Found";
	this->statusCodes[405] = "Method Not Allowed";
	this->statusCodes[413] = "Content Too Large";
	this->statusCodes[500] = "Internal Server Error";
	this->statusCodes[501] = "Not Implemented";
	this->statusCodes[504] = "Gateway Timeout";
	this->statusCodes[505] = "HTTP Version Not Supported";
}

void Config::initMimeTypes()
{
	this->mimeTypes[".html"] = "text/html";
	this->mimeTypes[".htm"] = "text/html";
	this->mimeTypes[".css"] = "text/css";
	this->mimeTypes[".js"] = "application/javascript";
	this->mimeTypes[".json"] = "application/json";
	this->mimeTypes[".png"] = "image/png";
	this->mimeTypes[".jpg"] = "image/jpeg";
	this->mimeTypes[".jpeg"] = "image/jpeg";
	this->mimeTypes[".gif"] = "image/gif";
	this->mimeTypes[".svg"] = "image/svg+xml";
	this->mimeTypes[".mp4"] = "video/mp4";
	this->mimeTypes[".webm"] = "video/webm";
	this->mimeTypes[".ogg"] = "video/ogg";
	this->mimeTypes[".mp3"] = "audio/mpeg";
	this->mimeTypes[".wav"] = "audio/wav";
	this->mimeTypes[".txt"] = "text/plain";
	this->mimeTypes[".xml"] = "application/xml";
	this->mimeTypes[".pdf"] = "application/pdf";
	this->mimeTypes[".zip"] = "application/zip";
	this->mimeTypes[".ico"] = "image/x-icon";
	this->mimeTypes[".ttf"] = "font/ttf";
	this->mimeTypes[".otf"] = "font/otf";
	this->mimeTypes[".woff"] = "font/woff";
	this->mimeTypes[".woff2"] = "font/woff2";
	this->mimeTypes[".eot"] = "font/eot";
	this->mimeTypes[".csv"] = "text/csv";
	this->mimeTypes[".doc"] = "application/msword";
}

Config::Config(const std::string &filePath)
{
	if (filePath.empty())
		throw std::runtime_error("File name can't be empty");

	std::ifstream configFile(filePath);
	if (!configFile.is_open())
		throw std::runtime_error("Failed to open config file");

	std::string line;
	this->lineNumber = 0;
	while (std::getline(configFile, line))
	{
		lineNumber++;
		this->trimWhitespace(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "server {")
			this->processServerBlock();
		else if (line.rfind("location ", 0) == 0 && line.back() == '{')
			this->processLocationBlock(line);
		else if (line == "}")
			this->processClosingBrace();
		else if (!blockStack.empty())
			this->handleKeyValue(line);
		else
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid syntax");
	}

	if (!blockStack.empty())
		throw std::runtime_error("Invalid config file: Unclosed blocks");
	if (this->servers.size() == 0)
		throw std::runtime_error("Invalid config file: No server is found");

	this->initStatusCodes();
	this->initMimeTypes();
}