/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/16 17:24:51 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

#include <vector>
#include <map>
#include <algorithm>

#define NAME_LENGHT 6

class   FileUpload {
    private:
        // utils variables
        std::string ContentDisposition;
        std::string FileNameString;
        std::string ContentType;
        std::string NameString;
        std::string substr;
        size_t pos;

        std::string Name;
        std::string FileName;
        std::string MimeType;
        int HeaderFetched;
        int DataFinish;
        int fd;
        bool FileNameEmpty;
        std::string generate_random_string(int length);
        void    ParseContentDisposition(std::string &Body);
        void    ParseContentType(std::string &Body);
        void    OpenFile(std::string &path);
        void    WriteToFile(std::string &Body, std::string &Boundary);
    public:
        FileUpload();
        ~FileUpload();
        void    ParseBody(std::string Body, std::string Boundary, std::string path);
};
