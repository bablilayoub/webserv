/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/02/16 16:08:46 by abablil          ###   ########.fr       */
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
    this->bytesLeft = 0;
    this->chunkSize = 0;

    this->DataFinish = false;

    // For Open or not The binary file
    this->BinaryFileOpen = false;

    // For Open or not the file
    this->openFile = false;

    // For Binary Chunked Data
    this->BinaryChunkSizeString = "";
    this->BinaryBytesLeft = 0;
    this->BinaryChunkData = "";
    this->BinarychunkSize = 0;
    this->BinaryDataFinish = false;

    // Mime Type Map
    this->MimeTypeMap["application/octet-stream"] = ".bin";
    this->MimeTypeMap["application/json"] = ".json";
    this->MimeTypeMap["application/xml"] = ".xml";
    this->MimeTypeMap["application/zip"] = ".zip";
    this->MimeTypeMap["application/gzip"] = ".gz";
    this->MimeTypeMap["application/x-tar"] = ".tar";
    this->MimeTypeMap["application/x-7z-compressed"] = ".7z";
    this->MimeTypeMap["application/pdf"] = ".pdf";
    this->MimeTypeMap["application/x-www-form-urlencoded"] = ".txt";
    this->MimeTypeMap["application/x-bzip"] = ".bz";
    this->MimeTypeMap["application/x-bzip2"] = ".bz2";
    this->MimeTypeMap["application/x-rar-compressed"] = ".rar";
    this->MimeTypeMap["application/x-msdownload"] = ".exe";
    this->MimeTypeMap["application/vnd.ms-excel"] = ".xls";
    this->MimeTypeMap["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx";
    this->MimeTypeMap["text/plain"] = ".txt";
    this->MimeTypeMap["text/html"] = ".html";
    this->MimeTypeMap["text/css"] = ".css";
    this->MimeTypeMap["text/csv"] = ".csv";
    this->MimeTypeMap["text/javascript"] = ".js";
    this->MimeTypeMap["application/javascript"] = ".js";
    this->MimeTypeMap["image/jpeg"] = ".jpg";
    this->MimeTypeMap["image/png"] = ".png";
    this->MimeTypeMap["image/gif"] = ".gif";
    this->MimeTypeMap["image/svg+xml"] = ".svg";
    this->MimeTypeMap["image/webp"] = ".webp";
    this->MimeTypeMap["image/bmp"] = ".bmp";
    this->MimeTypeMap["audio/mpeg"] = ".mp3";
    this->MimeTypeMap["audio/wav"] = ".wav";
    this->MimeTypeMap["audio/ogg"] = ".ogg";
    this->MimeTypeMap["video/mp4"] = ".mp4";
    this->MimeTypeMap["video/x-msvideo"] = ".avi";
    this->MimeTypeMap["video/webm"] = ".webm";
    this->MimeTypeMap["video/quicktime"] = ".mov";
    this->MimeTypeMap["video/x-flv"] = ".flv";
}

FileUpload::~FileUpload()
{
    if (this->fd > 0)
        close(this->fd);
}

void FileUpload::ResetData()
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

void FileUpload::ParseContentDisposition(std::string &Body)
{
    Body = Body.substr(this->pos + this->ContentDisposition.length() + 1, Body.length());
    this->pos = Body.find(this->NameString);
    Body = Body.substr(this->pos + this->NameString.length(), Body.length());
    this->substr = Body.substr(0, Body.find("\""));
    Body = Body.substr(Body.find("\"") + 3, Body.length());
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
        this->substr = Body.substr(0, Body.find("\""));
        Body = Body.substr(Body.find("\"") + 3, Body.length());
        this->FileName = substr;
        if (this->FileName.empty())
            this->FileNameEmpty = true;
    }
    this->HeaderFetched = true;
}

void FileUpload::ParseContentType(std::string &Body)
{
    Body = Body.substr(this->pos + this->ContentType.length() + 1, Body.length());
    this->pos = Body.find("/");
    Body = Body.substr(this->pos + 1, Body.length());
    this->MimeType = Body.substr(0, Body.find("\n") - 1);
    Body = Body.substr(Body.find("\n") + 3, Body.length());
    if (this->FileNameEmpty)
    {
        if (this->MimeTypeMap.find(this->MimeType) != this->MimeTypeMap.end())
            this->FileName = generate_random_string(5) + this->MimeTypeMap[this->MimeType];
        else
            this->FileName = generate_random_string(5) + ".bin";
    }
    this->HeaderFetched = true;
}

void FileUpload::OpenFile(std::string path)
{
    if (this->HeaderFetched)
    {
        this->HeaderFetched = false;
        if (this->fd > 0)
            close(this->fd);
        this->fd = -42;
        if (!this->FileName.empty())
        {
            std::string OpenPath = path + "/" + this->FileName;
            this->fd = open(OpenPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (this->fd < 0)
            {
                std::cerr << "Failed to open the file : " << OpenPath << std::endl;
                return;
            }
        }
    }
}

void FileUpload::WriteToFile(std::string &Body)
{
    if (this->fd > 0)
    {
        if (Body.empty() || (Body.length() == 2 && Body == "\r\n"))
            return;
        write(this->fd, Body.c_str(), Body.size());
    }
}

int FileUpload::HandleChunkedData(std::string &Body)
{
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
        pos = Body.find("0\r\n\r\n");
        if (pos != std::string::npos)
        {
            if (pos != 0)
            {
                chunkData = Body.substr(0, pos);
                if (!chunkData.empty())
                    this->WriteToFile(chunkData);
            }
            this->DataFinish = true;
            return 1;
        }
        pos = Body.find(CRLF);

        if (pos == 0)
        {
            Body = Body.substr(2);
            continue;
        }
        if (pos == std::string::npos)
            break;

        ChunkSizeString = Body.substr(0, pos);
        std::istringstream iss(ChunkSizeString);
        chunkSize = 0;
        iss >> std::hex >> chunkSize;

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
    return 0;
}

void FileUpload::HandleBinaryData(std::string mimeType)
{
    size_t posMime;

    if ((posMime = mimeType.find("/")) != std::string::npos)
    {

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

int FileUpload::HandleBinaryChunkedData(std::string &Body)
{
    while (true && !this->BinaryDataFinish)
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

            if (BinarychunkSize == 0)
            {
                this->BinaryDataFinish = true;
                return 1;
                break;
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

            if (!BinaryChunkData.empty())
                this->WriteToFile(BinaryChunkData);
        }
        else
            break;

        if (Body.empty())
            break;
    }
    return 0;
}

int FileUpload::ParseBody(std::string Body, std::string Boundary, Client &client)
{
    if (Body.find(Boundary + "--") != std::string::npos && !client.getIsBinary())
    {
        this->DataFinish = true;
        return 2;
    }
    if (this->DataFinish || (Body.empty() && !client.getIsBinary()))
        return 2;
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
    {
        if (this->HandleChunkedData(Body))
            return 1;
    }
    else if (client.getIsChunked() && client.getIsBinary())
    {
        if (this->HandleBinaryChunkedData(Body) == 1)
            return 1;
    }
    else
        this->WriteToFile(Body);

    return 0;
}
