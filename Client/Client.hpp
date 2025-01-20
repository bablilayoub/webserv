/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 10:59:39 by abablil           #+#    #+#             */
/*   Updated: 2025/01/20 12:19:19 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Global.hpp"
#include "Config.hpp"

struct Response
{
	int statusCode;
	std::string statusMessage;
	std::string content;
	std::string contentType;
};

class Client
{
private:
	int port;
	int clientFd;
	int content_length;
	bool isChunked;
	bool isContentLenght;
	bool isBinary;
	std::string server_name;
	std::string path;
	std::string method;
	std::string body;
	std::string boundary;
	std::string content_type;
	std::map<std::string, std::string> headers;

	Config *config;
	std::string upload_dir;

	std::string responseString;
	Response response;

	void clear();

	void handleFirstLine(std::istringstream &requestStream);
	void generateResponse();
	void checkConfigs();

	Server *getServer();
	Location *getLocation();
	
	std::string loadFile(const std::string &filePath);
	std::string loadFiles(const std::string &directory);
	std::string loadErrorPage(const std::string &filePath, int statusCode);
	std::string getErrorPagePath(int errorCode);

	std::string getMimeType(const std::string &path);
	void logRequest(int statusCode);

	void handleCGIRequest(const std::string &path);
	bool isCGIRequest(const std::string &path);
	void setErrorResponse(int statusCode);
	void setSuccessResponse(int statusCode, const std::string &path);
	
public:
	void setup(int fd, Config *config);
	void parse(const std::string &request);
	const std::string &getBody() const;
	const std::string &getBoundary() const;
	const std::string &getMethod() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const int &getContentLength() const;
	const bool &getIsChunked() const;
	const bool &getIsBinary() const;
	const bool &getIsContentLenght() const;
	const std::string &getResponse() const;
	const std::string &getUploadDir() const;
	const std::string &getContentType() const;
};
