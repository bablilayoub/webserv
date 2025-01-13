/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/13 18:37:59 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() { this->firstChunk = true; }

void Client::handleFirstLine(std::istringstream &requestStream)
{
	std::string line;
	std::getline(requestStream, line);

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

void Client::parse(const std::string &request, std::map<int, FileUpload>& BodyMap)
{
	std::istringstream requestStream(request);

	if (this->firstChunk)
		this->handleFirstLine(requestStream);
		
	std::string line;
	while (std::getline(requestStream, line))
	{
		if (this->firstChunk)
		{
			size_t boundaryPos = line.find(BOUNDARY_PREFIX);
			size_t contentLengthPos = line.find(CONTENT_LENGTH_PREFIX);
			size_t hostPrefixPos = line.find(HOST_PREFIX);
			size_t bodyPrefixPos = std::string::npos;
			size_t colonPos = line.find(':');
			if (!this->boundary.empty())
				bodyPrefixPos = line.find(this->boundary);

			if (boundaryPos != std::string::npos)
			{
				boundaryPos += std::string(BOUNDARY_PREFIX).length();
				this->boundary = line.substr(boundaryPos, line.size() - boundaryPos);
			}
			else if (contentLengthPos != std::string::npos)
			{
				contentLengthPos += std::string(CONTENT_LENGTH_PREFIX).length();
				this->content_length = std::atof(line.substr(contentLengthPos).c_str());
			}
			else if (hostPrefixPos != std::string::npos)
			{
				std::string hostPrefix(HOST_PREFIX);
				size_t colonPos = line.find(':', hostPrefix.length());
				if (colonPos == std::string::npos)
					throw std::runtime_error("Invalid host format");

				this->ip = line.substr(hostPrefix.length(), colonPos - hostPrefix.length());
				this->port = std::atof(line.substr(colonPos + 1).c_str());
			}
			else if (!this->boundary.empty() && bodyPrefixPos != std::string::npos)
			{
				this->body += line + '\n';
				while (std::getline(requestStream, line) && line.find(this->boundary + "--") == std::string::npos)
					this->body += line + '\n';
				bodyPrefixPos = std::string::npos;
			}
			else if (colonPos != std::string::npos)
			{
				this->headers[line.substr(0, colonPos)] = line.substr(colonPos + 2, line.size());
			}
		}
		else
		{
			this->body.clear();
			while (std::getline(requestStream, line))
				this->body += line + '\n';
		}
	}

	

	if (this->firstChunk)
		this->firstChunk = false;

	// std::cout << this->ip << std::endl;
	// std::cout << this->port << std::endl;
	// std::cout << this->method << std::endl;
	// std::cout << this->boundary << std::endl;
	// std::cout << this->headers << std::endl;
	// std::cout << "|============|" << std::endl;
	// std::cout << this->body << std::endl;
	BodyMap[this->clientFd].ParseBody(this->body, this->boundary);
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
