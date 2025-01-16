/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:49:18 by abablil           #+#    #+#             */
/*   Updated: 2025/01/16 12:44:45 by abablil          ###   ########.fr       */
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
		throw std::runtime_error("Invalid key-value pair: Missing space between key and value");

	key = line.substr(0, pos);
	value = line.substr(pos + 1);

	if (value.back() != ';')
		throw std::runtime_error("Invalid key-value pair: Missing semicolon at the end");

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
		this->trimWhitespace(locationPath);
		if (currentLocation.root_folder.empty())
			currentLocation.root_folder = currentServer.root_folder;
		if (std::find(currentLocation.accepted_methods.begin(), currentLocation.accepted_methods.end(), METHOD_POST) != currentLocation.accepted_methods.end())
			if (currentLocation.upload_dir.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": An upload directory is required for the POST method.");
		currentServer.locations[locationPath] = currentLocation;
		locationPath.clear();
	}
	else if (closingBlock == "server")
	{
		if (currentServer.root_folder.empty())
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Root folder is not specified");
		if (currentServer.listen_port < 1 || currentServer.listen_port > 65535)
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid port , Allowed ports (1 to 65535)");
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

void Config::handleKeyValue(const std::string &line)
{
	std::string key, value;
	this->parseKeyValue(line, key, value);

	if (blockStack.top() == "server")
	{
		if (key == "listen")
			currentServer.listen_port = this->parseInt(value);
		else if (key == "server_names")
		{
			currentServer.server_names.clear();
			std::istringstream stream(value);
			std::string server_name;

			while (std::getline(stream, server_name, ' '))
			{
				this->trimWhitespace(server_name);
				currentServer.server_names.push_back(server_name);
			}

			if (currentServer.server_names.empty())
				throw std::runtime_error("Line " + std::to_string(lineNumber) + ": server_names cannot be empty");
		}
		else if (key == "limit_client_body_size")
			currentServer.limit_client_body_size = value;
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
			currentLocation.upload_dir = value;
		else if (key == "redirect")
			currentLocation.redirect = value;
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
		else
			throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Unknown location key: " + key);
	}
	else
		throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid syntax");
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
}