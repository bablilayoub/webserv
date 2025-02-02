/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/22 12:12:00 by aitaouss         ###   ########.fr       */
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

#define CRLF "\r\n"

class Client;
class   FileUpload 
{
    private:
        std::map<std::string, std::string>  MimeTypeMap;

        std::string ChunkSizeString;
        std::string chunkData;
        bool        ChunkDone;  
        bool        FirstChunk;
        size_t      bytesLeft;
        size_t      chunkSize;

        std::string BinaryChunkData;      
        size_t      BinarychunkSize;
        size_t      BinaryBytesLeft;
        std::string BinaryChunkSizeString;
        bool        BinaryDataFinish;

        bool        DataFinish;

        std::string ContentDisposition;
        std::string FileNameString;
        std::string ContentType;
        std::string NameString;
        std::string substr;
        size_t      pos;

        std::string Name;
        std::string FileName;
        std::string MimeType;
        int         fd;
        int         HeaderFetched;
        bool        BinaryFileOpen;
        bool        openFile;
        bool        FileNameEmpty;

        std::string generate_random_string(int length);
        void        ParseContentDisposition(std::string &Body);
        void        ParseContentType(std::string &Body);
        void        OpenFile(std::string path);
        void        WriteToFile(std::string &Body);
        int         HandleChunkedData(std::string &Body);
        void        ResetData();

    public:
        FileUpload();
        ~FileUpload();
        int        ParseBody(std::string Body, std::string Boundary, Client &client);
        void        HandleBinaryData(std::string mimeType);
        int        HandleBinaryChunkedData(std::string &Body);
};

