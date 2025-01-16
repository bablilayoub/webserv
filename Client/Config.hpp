/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 09:07:41 by abablil           #+#    #+#             */
/*   Updated: 2025/01/16 12:42:35 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Global.hpp"

struct Location
{
    bool autoindex;
    std::string redirect;
    std::string upload_dir;
    std::string root_folder;
    std::string index;
    std::vector<std::string> accepted_methods;
};

struct Server
{
    int listen_port;
    std::string limit_client_body_size;
    std::vector<std::string> server_names;
    std::string root_folder;
    std::map<int, std::string> error_pages;
    std::map<std::string, Location> locations;
};

class Config
{
private:
    std::ifstream configFile;

    int lineNumber;
    Server currentServer;
    Location currentLocation;
    std::string locationPath;
    std::stack<std::string> blockStack;

    void parseKeyValue(const std::string &line, std::string &key, std::string &value);
    int parseInt(const std::string &value);
    void processServerBlock();
    void processLocationBlock(const std::string &line);
    void processClosingBrace();
    void handleKeyValue(const std::string &line);
    void trimWhitespace(std::string &line);
    std::string trimTrailingSlash(const std::string &path);
    bool isValidDirectory(const std::string &path);

public:
    std::vector<Server> servers;

    Config(const std::string &filePath);
    ~Config();
};
