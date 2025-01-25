/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/22 14:42:28 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"
#include "../Client/Client.hpp"

FileUpload::FileUpload() 
{
    this->HeaderFetched = false;
    this->fd = -42;
    this->Name = "";
    this->FileName = "";
    this->MimeType = "";
    this->ContentDisposition = "Content-Disposition: form-data;";
    this->FileNameString = "filename=\"";
    this->ContentType = "Content-Type:";
    this->NameString = "name=\"";
    this->substr = "";
    this->pos = -42;
    this->FileNameEmpty = false;

    // For the FormData Chunked
    this->ChunkSizeString = "";
    this->FirstChunk = true;
    this->ChunkDone = false;
    this->FirstCRLF = true;
    this->bytesLeft = 0;
    this->chunkSize = 0;

    this->DataFinish = false;

    // For the Open or not The binary file
    this->BinaryFileOpen = false;

    this->openFile = false;

    // For Binary Chunked Data
    this->BinaryChunkSizeString = "";
    this->BinaryBytesLeft = 0;
    this->BinaryChunkData = "";
    this->BinarychunkSize = 0;
    this->BinaryDataFinish = false;

    // for the raw data
    this->MimeTypeMap["application/octet-stream"] = ".bin";  // Generic binary data
    this->MimeTypeMap["application/json"] = ".json";        // JSON format
    this->MimeTypeMap["application/xml"] = ".xml";          // XML format
    this->MimeTypeMap["application/zip"] = ".zip";          // ZIP archives
    this->MimeTypeMap["application/gzip"] = ".gz";          // Gzip archives
    this->MimeTypeMap["application/x-tar"] = ".tar";        // Tar archives
    this->MimeTypeMap["application/x-7z-compressed"] = ".7z"; // 7-Zip archives
    this->MimeTypeMap["application/pdf"] = ".pdf";          // PDF documents
    this->MimeTypeMap["application/x-www-form-urlencoded"] = ".txt"; // Form data
    this->MimeTypeMap["application/x-bzip"] = ".bz";        // Bzip archives
    this->MimeTypeMap["application/x-bzip2"] = ".bz2";      // Bzip2 archives
    this->MimeTypeMap["application/x-rar-compressed"] = ".rar"; // RAR archives
    this->MimeTypeMap["application/x-msdownload"] = ".exe"; // Windows executables
    this->MimeTypeMap["application/vnd.ms-excel"] = ".xls"; // Excel spreadsheets
    this->MimeTypeMap["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx"; // Excel (modern)

    this->MimeTypeMap["text/plain"] = ".txt";               // Plain text
    this->MimeTypeMap["text/html"] = ".html";               // HTML documents
    this->MimeTypeMap["text/css"] = ".css";                 // CSS stylesheets
    this->MimeTypeMap["text/csv"] = ".csv";                 // CSV files
    this->MimeTypeMap["text/javascript"] = ".js";           // JavaScript
    this->MimeTypeMap["application/javascript"] = ".js";    // Modern JavaScript

    this->MimeTypeMap["image/jpeg"] = ".jpg";               // JPEG images
    this->MimeTypeMap["image/png"] = ".png";                // PNG images
    this->MimeTypeMap["image/gif"] = ".gif";                // GIF images
    this->MimeTypeMap["image/svg+xml"] = ".svg";            // SVG images
    this->MimeTypeMap["image/webp"] = ".webp";              // WebP images
    this->MimeTypeMap["image/bmp"] = ".bmp";                // BMP images

    this->MimeTypeMap["audio/mpeg"] = ".mp3";               // MP3 audio
    this->MimeTypeMap["audio/wav"] = ".wav";                // WAV audio
    this->MimeTypeMap["audio/ogg"] = ".ogg";                // Ogg Vorbis audio

    this->MimeTypeMap["video/mp4"] = ".mp4";                // MP4 video
    this->MimeTypeMap["video/x-msvideo"] = ".avi";          // AVI video
    this->MimeTypeMap["video/webm"] = ".webm";              // WebM video
    this->MimeTypeMap["video/quicktime"] = ".mov";          // MOV video
    this->MimeTypeMap["video/x-flv"] = ".flv";              // Flash Video



}

FileUpload::~FileUpload() 
{
    if (this->fd > 0)
        close(this->fd);
}

void    FileUpload::ResetData() 
{
    this->HeaderFetched = false;
    this->fd = -42;
    this->Name = "";
    this->FileName = "";
    this->MimeType = "";
    this->ContentDisposition = "Content-Disposition: form-data;";
    this->FileNameString = "filename=\"";
    this->ContentType = "Content-Type:";
    this->NameString = "name=\"";
    this->substr = "";
    this->pos = -42;
    this->FileNameEmpty = false;

    this->ChunkSizeString = "";
    this->FirstChunk = true;
    this->ChunkDone = false;
    this->FirstCRLF = true;
    this->bytesLeft = 0;
    this->chunkSize = 0;

    this->DataFinish = false;

    this->BinaryFileOpen = false;

    this->openFile = false;
}

std::string FileUpload::generate_random_string(int length) 
{
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string newstr;

    static unsigned int seed_counter = 0;
    unsigned int seed = static_cast<unsigned int>(std::time(0) + (++seed_counter));
    std::srand(seed);

    for (int i = 0; i < length; i++)
        newstr += str[rand() % str.size()];

    return newstr;
}

void    FileUpload::ParseContentDisposition(std::string &Body) 
{
    Body = Body.substr(this->pos + this->ContentDisposition.length() + 1, Body.length());
    this->pos = Body.find(this->NameString);
    Body = Body.substr(this->pos + this->NameString.length(), Body.length());
    this->substr = Body.substr(0, Body.find("\""));
    Body = Body.substr(Body.find("\"") + 3 , Body.length());
    this->Name = substr;

    this->pos = Body.find(this->FileNameString);
    if (this->pos == std::string::npos) 
    {
        this->FileNameEmpty = true;
        this->FileName = "RBL" + generate_random_string(5);
        Body = Body.substr(2, Body.length());
    }
    else if (this->pos != std::string::npos) 
    {
        this->FileNameEmpty = false;
        Body = Body.substr(this->pos + this->FileNameString.length(), Body.length());
        this->substr = Body.substr(0 , Body.find("\""));
        Body = Body.substr(Body.find("\"") + 3 , Body.length());
        this->FileName = substr;
        if (this->FileName.empty())
            this->FileNameEmpty = true;
    }
    this->HeaderFetched = true;
}

void    FileUpload::ParseContentType(std::string &Body) 
{
    Body = Body.substr(this->pos + this->ContentType.length() + 1, Body.length());
    this->pos = Body.find("/");
    Body = Body.substr(this->pos + 1, Body.length());
    this->MimeType = Body.substr(0, Body.find("\n") - 1);
    Body = Body.substr(Body.find("\n") + 3 , Body.length());
    if (this->FileNameEmpty) 
        this->FileName = this->FileName + "." + this->MimeType;
    this->HeaderFetched = true;
}

void    FileUpload::OpenFile(std::string path) 
{
    if (this->HeaderFetched)
    {
        std::cout << "Header Fetched Open FIle : " << FileName << std::endl;
        this->HeaderFetched = false;
        close(this->fd);
        this->fd = -42;
        if (!this->FileName.empty()) 
        {
            std::string OpenPath = path + "/" + this->FileName;
            this->fd = open(OpenPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (this->fd < 0)
            {
                std::cout << "Failed to open the file : " << OpenPath << std::endl;
                return ;
            }
        }
    }
}

void    FileUpload::WriteToFile(std::string &Body) {
    if (this->fd > 0) 
    {
        if (Body.empty() || (Body.length() == 2 && Body == "\r\n"))
            return ;
        write(this->fd, Body.c_str(), Body.size());
    }
}

void FileUpload::HandleChunkedData(std::string &Body) {
    while (true) 
    {
        if (Body.length() == 2 && Body == CRLF)
            break;

        if (bytesLeft > 0 && Body.length() >= bytesLeft) 
        {
            chunkData = Body.substr(0, bytesLeft);

            if (!chunkData.empty())
                this->WriteToFile(chunkData);

            Body = Body.substr(bytesLeft);
            bytesLeft = 0;
        } 
        else if (bytesLeft > 0) 
        {
            chunkData = Body;

            if (!chunkData.empty())
                this->WriteToFile(chunkData);

            bytesLeft -= chunkData.length();
            Body.clear();
        }

        pos = Body.find(CRLF);
        if (pos == std::string::npos)
            break;

        ChunkSizeString = Body.substr(0, pos);
        std::istringstream iss(ChunkSizeString);
        chunkSize = 0;
        iss >> std::hex >> chunkSize;

        // std::cout << "Chunk string : " << ChunkSizeString << std::endl;
        // std::cout << "Chunk size : " << chunkSize << std::endl;

        if (pos + 2 + chunkSize > Body.length()) 
        {
            bytesLeft = chunkSize - (Body.length() - pos - 2);
            chunkData = Body.substr(pos + 2);
            Body.clear();
        } 
        else 
        {
            chunkData = Body.substr(pos + 2, chunkSize);
            Body = Body.substr(pos + 2 + chunkSize);
            bytesLeft = 0;
        }

        if (!chunkData.empty())
            this->WriteToFile(chunkData);

        if (Body.empty())
            break;
    }
}

void FileUpload::HandleBinaryData(std::string mimeType) {
    size_t posMime;

    if ((posMime = mimeType.find("/")) != std::string::npos) {

        if (this->MimeTypeMap.find(mimeType) != this->MimeTypeMap.end())
            mimeType = this->MimeTypeMap[mimeType];
        else 
            mimeType = ".bin";

        this->FileName = "RBL" + generate_random_string(5) + mimeType;

        this->BinaryFileOpen = true;
        this->HeaderFetched = true;
        this->openFile = true;

    }
}

void    FileUpload::HandleBinaryChunkedData(std::string &Body) {
    while (true && !this->DataFinish) 
    {
        if (Body.length() == 2 && Body == CRLF)
            break;
        if (BinaryBytesLeft > 0 && Body.length() >= BinaryBytesLeft) 
        {
            BinaryChunkData = Body.substr(0, BinaryBytesLeft);

            if (!BinaryChunkData.empty())
                this->WriteToFile(BinaryChunkData);

            Body = Body.substr(BinaryBytesLeft);
            BinaryBytesLeft = 0;
        } 
        else if (BinaryBytesLeft > 0) 
        {
            BinaryChunkData = Body;

            if (!BinaryChunkData.empty())
                this->WriteToFile(BinaryChunkData);

            BinaryBytesLeft -= BinaryChunkData.length();
            Body.clear();
        }
        this->pos = Body.find(CRLF);
        if (this->pos == 0)
            Body = Body.substr(2);
        else if (this->pos != std::string::npos) 
        {
            this->BinaryChunkSizeString = Body.substr(0, pos);
            std::istringstream iss(BinaryChunkSizeString);
            this->BinarychunkSize = 0;
            iss >> std::hex >> BinarychunkSize;


            if (BinarychunkSize == 0) {
                this->BinaryDataFinish = true;
            }
            if (pos + 2 + BinarychunkSize > Body.length()) 
            {
                BinaryBytesLeft = BinarychunkSize - (Body.length() - pos - 2);
                BinaryChunkData = Body.substr(pos + 2);
                Body.clear();
            } 
            else 
            {
                BinaryChunkData = Body.substr(pos + 2, BinarychunkSize);
                Body = Body.substr(pos + 2 + BinarychunkSize);
                BinaryBytesLeft = 0;
            }
            std::cout << "Chunk size: " << BinarychunkSize << ", Bytes written: " << BinaryChunkData.length() << std::endl;
            std::cout << "Bytes left to write: " << BinaryBytesLeft << std::endl;

            if (!BinaryChunkData.empty()) 
                this->WriteToFile(BinaryChunkData);

        }
        else
            break;

        if (Body.empty())
            break;
    }
}

void    FileUpload::ParseBody(std::string Body, std::string Boundary, Client &client)
{
    if ((Body.length() == 2 && Body == CRLF) || (Body.length() == 1 && Body == "\n") || (Body.length() == 1 && Body == "\r"))
        return ;
    if (Body.find(Boundary + "--") != std::string::npos && !client.getIsBinary()) 
    {
        this->DataFinish = true;
        return ;
    }
    if (this->DataFinish || Body.empty())
        return ;
    if (client.getIsBinary() && !this->BinaryFileOpen) 
    {
        this->BinaryChunkData = "";
        this->BinaryBytesLeft = 0;
        this->BinarychunkSize = 0;
        HandleBinaryData(client.getContentType());
    }
    else
    {
        this->pos = Body.find(Boundary);
        if (this->pos == 0) 
        {
            this->openFile = false;
            this->chunkData = "";
            this->bytesLeft = 0;
            this->chunkSize = 0;
            this->pos = Body.find(this->ContentDisposition);
            if (this->pos != std::string::npos)
                this->ParseContentDisposition(Body);
            this->pos = Body.find(this->ContentType);
            if (this->pos != std::string::npos) 
            {
                this->openFile = true;
                this->ParseContentType(Body);
            }
        }
    }

    if (this->openFile)
        this->OpenFile(client.getUploadDir());

    if (client.getIsChunked() && !client.getIsBinary())
        this->HandleChunkedData(Body);
    else if (client.getIsChunked() && client.getIsBinary())
        this->HandleBinaryChunkedData(Body);
    else
        this->WriteToFile(Body);
}

// 574988518
// 574947328