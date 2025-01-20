/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/20 16:16:26 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {

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

    this->IsChunked = false;
    this->ChunkSizeString = "";
    this->FirstChunk = true;
    this->ChunkDone = false;
    this->FirstCRLF = true;
    this->bytesLeft = 0;
    this->chunkSize = 0;

    this->BinaryFileOpen = false;
    this->IsBinary = false;

}

FileUpload::~FileUpload() {
    if (this->fd > 0) {
        close(this->fd);
    }
}

std::string FileUpload::generate_random_string(int length) {
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string newstr;

    static unsigned int seed_counter = 0;
    unsigned int seed = static_cast<unsigned int>(std::time(0) + (++seed_counter));
    std::srand(seed);

    for (int i = 0; i < length; i++) {
        newstr += str[rand() % str.size()];
    }
    return newstr;
}

void    FileUpload::ParseContentDisposition(std::string &Body) {
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

void    FileUpload::ParseContentType(std::string &Body) {
    Body = Body.substr(this->pos + this->ContentType.length() + 1, Body.length());
    this->pos = Body.find("/");
    Body = Body.substr(this->pos + 1, Body.length());
    this->MimeType = Body.substr(0, Body.find("\n") - 1);
    Body = Body.substr(Body.find("\n") + 3 , Body.length());
    if (this->FileNameEmpty) 
        this->FileName = this->FileName + "." + this->MimeType;
    this->HeaderFetched = true;
}

void    FileUpload::OpenFile(std::string &path) {
    if (this->HeaderFetched)
    {
        std::cout << "Header Fetched Open FIle : " << FileName << std::endl;
        this->HeaderFetched = false;
        close(this->fd);
        this->fd = -42;
        if (!this->FileName.empty()) 
        {
            std::string OpenPath = path + "/" + this->FileName;
            this->fd = open(OpenPath.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
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
        if (Body.empty())
            return ;
        write(this->fd, Body.c_str(), Body.size());
    }
}

// Not Use For now
void eraseBody(std::string &Body, const std::string &StringtoErase) {
    if (StringtoErase.empty()) {
        return;
    }
    size_t pos;
    if ((pos = Body.find(StringtoErase)) != std::string::npos)
        Body.erase(pos, StringtoErase.length());
}

// void    FileUpload::HandleChunkedData(std::string &Body) {
//     while (true) 
//     {
//         if (bytesLeft > 0 && Body.length() >= bytesLeft) {
//             chunkData = Body.substr(0, bytesLeft);
//             if (!chunkData.empty()) {
//                 this->WriteToFile(chunkData);
//             }
//             Body = Body.substr(chunkData.length(), Body.length());
//             bytesLeft = 0;
//         }
//         else {
//             if (bytesLeft > 0) {
//                 chunkData = Body;
//                 if (!chunkData.empty()) {
//                     this->WriteToFile(chunkData);
//                 }
//                 bytesLeft = bytesLeft - chunkData.length();
//                 Body = "";
//             }
//         }
//         pos = Body.find(CRLF);
//         ChunkSizeString = Body.substr(0, pos);
//         std::istringstream iss(ChunkSizeString);
//         chunkSize = 0;
//         iss >> std::hex >> chunkSize;
//         chunkData = Body.substr(pos + 2, chunkSize);
//         if (chunkData.length() < chunkSize) 
//         {
//             bytesLeft = chunkSize - chunkData.length();
//         }
//         Body = Body.substr(pos + 2 + chunkData.length(), Body.length());
        
//         if (!chunkData.empty()) {
//             this->WriteToFile(chunkData);
//         }
//         if (Body.length() == 0) {
//             break;
//         }
//     }
// }

void FileUpload::HandleChunkedData(std::string &Body) {
    while (true) {
        if (bytesLeft > 0 && Body.length() >= bytesLeft) {
            chunkData = Body.substr(0, bytesLeft);
            if (!chunkData.empty()) {
                this->WriteToFile(chunkData);
            }
            Body = Body.substr(bytesLeft);
            bytesLeft = 0;
        } else if (bytesLeft > 0) {
            chunkData = Body;
            if (!chunkData.empty()) {
                this->WriteToFile(chunkData);
            }
            bytesLeft -= chunkData.length();
            Body.clear();
        }

        pos = Body.find(CRLF);
        if (pos == std::string::npos) {
            break;
        }

        ChunkSizeString = Body.substr(0, pos);
        std::istringstream iss(ChunkSizeString);
        chunkSize = 0;
        iss >> std::hex >> chunkSize;

        if (pos + 2 + chunkSize > Body.length()) {
            bytesLeft = chunkSize - (Body.length() - pos - 2);
            chunkData = Body.substr(pos + 2);
            Body.clear();
        } 
        else {
            chunkData = Body.substr(pos + 2, chunkSize);
            Body = Body.substr(pos + 2 + chunkSize);
            bytesLeft = 0;
        }

        if (!chunkData.empty()) {
            this->WriteToFile(chunkData);
        }

        if (Body.empty()) {
            break;
        }
    }
}

void    FileUpload::HandleBinaryData() {
    this->FileName = "RBL" + generate_random_string(5);
    this->BinaryFileOpen = true;
    this->HeaderFetched = true;
}

void    FileUpload::ParseBody(std::string Body, std::string Boundary, std::string path) 
{
    // std::cout << Body << std::endl;
    // std::cout << Body.length() << std::endl;
    // std::cout << " ------- Body------- " << std::endl;
    // return ;
    std::cout << "1" << std::endl;
    if (Body.empty()) {
        std::cout << "Body Empty" << std::endl;
        return ;
    }
    if (Body.find(Boundary + "--") != std::string::npos && !this->IsBinary)
        return ;
    

    if (this->IsBinary && !this->BinaryFileOpen) 
    {
        HandleBinaryData();
    }
    else
    {
        this->pos = Body.find(Boundary);
        if (this->pos == 0) 
        {
            this->chunkData = "";
            this->bytesLeft = 0;
            this->chunkSize = 0;
            this->pos = Body.find(this->ContentDisposition);
            if (this->pos != std::string::npos)
                this->ParseContentDisposition(Body);
            this->pos = Body.find(this->ContentType);
            if (this->pos != std::string::npos)
                this->ParseContentType(Body);
        }
    }

    this->OpenFile(path);

    if (this->IsChunked)
        this->HandleChunkedData(Body);
    else
        this->WriteToFile(Body);
}
