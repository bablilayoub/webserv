/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 10:59:39 by abablil           #+#    #+#             */
/*   Updated: 2025/01/11 15:18:53 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <map>

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"
#define BOUNDARY_PREFIX "boundary="
#define CONTENT_LENGTH_PREFIX "Content-Length: "
#define HOST_PREFIX "Host: "

class Client
{
private:
	int clientFd;
	int port;
	int content_length;
	bool firstChunk;
	std::string ip;
	std::string path;
	std::string method;
	std::string body;
	std::string boundary;
	std::map<std::string, std::string> headers;
	
	void handleFirstLine(std::istringstream &requestStream);
	void clear();

public:
	Client(int clientFd);
	void parse(const std::string &request);
	const std::string &getBody() const;
	const std::string &getBoundary() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const int &getContentLength() const;
};
