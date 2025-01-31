/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 10:59:39 by abablil           #+#    #+#             */
/*   Updated: 2025/01/29 12:23:05 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Global.hpp"
#include "Config.hpp"

struct Response
{
	int statusCode;
	std::string content;
	std::string contentType;
	std::string filePath;
	size_t totalSize;
	size_t sentSize;
	bool headers_sent;
	std::string headers;
	size_t lastReadPos;
	bool done;
};

struct CGIState {
    pid_t pid;
    std::string outputPath;
    std::string inputPath;
	bool running;
};

class Client
{
private:
	int port;
	int clientFd;
	size_t content_length;
	
	bool isCGI;
	bool isBinary;
	bool isChunked;
	bool isContentLenght;
	bool generated;

	std::string path;
	std::string sub_path;
	std::string body;
	std::string query;
	std::string method;
	std::string boundary;
	std::string server_name;
	std::string content_type;
	std::string path_info;

	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> cgi_response_headers;

	Config *config;
	std::string upload_dir;

	std::string responseString;
	CGIState cgi_state;

	void clear();

	void handleFirstLine(std::istringstream &requestStream);
	void startProcessing();

	Server *getServer();
	Location *getLocation();
	
	Location *location;

	std::string loadFile(const std::string &filePath);
	std::string loadFiles(const std::string &directory);
	std::string loadErrorPage(const std::string &filePath, int statusCode);
	std::string getErrorPagePath(int errorCode);

	std::string getMimeType(const std::string &path);
	void logRequest(int statusCode);

	void handleCGIRequest(const std::string &path);
	bool isCGIRequest();
	void setErrorResponse(int statusCode);
	void setSuccessResponse(int statusCode, const std::string &path);
	std::string getHttpHeaders();

	bool fileExists(const std::string &path);
	bool isDirectory(const std::string &path);
	bool hasReadPermission(const std::string &path);
	std::string urlDecode(const std::string &str);
	void setFinalResponse();
public:
	Server *server;
	bool return_anyway;
	Response response;

	void setup(int fd, Config *config);
	void parse(const std::string &request);
	void generateResponse();
	const std::string &getBody() const;
	const std::string &getBoundary() const;
	const std::string &getMethod() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const size_t &getContentLength() const;
	const bool &getIsChunked() const;
	const bool &getIsBinary() const;
	const bool &getIsContentLenght() const;
	const bool &getIsCGI() const;
	bool sendResponse();
	const std::string &getUploadDir() const;
	const std::string &getContentType() const;
	bool checkCGICompletion();
};
