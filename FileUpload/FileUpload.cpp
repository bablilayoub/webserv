/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/16 17:23:24 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->HeaderFetched = false;
    this->fd = -42;
    this->Name = "";
    this->FileName = "";
    this->MimeType = "";
    this->DataFinish = false;
    this->ContentDisposition = "Content-Disposition: form-data;";
    this->FileNameString = "filename=\"";
    this->ContentType = "Content-Type:";
    this->NameString = "name=\"";
    this->substr = "";
    this->pos = -42;
    this->FileNameEmpty = false;
}

FileUpload::~FileUpload() {
    if (this->fd > 0) {
        close(this->fd);
    }
}

std::string FileUpload::generate_random_string(int length) {
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string newstr;

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
        unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::srand(seed);
        this->FileName = "RBL" + generate_random_string(5);
        Body = Body.substr(2, Body.length());
    }
    if (this->pos != std::string::npos) 
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

void    FileUpload::WriteToFile(std::string &Body, std::string &Boundary) {
    if (Body.find(Boundary + "--") != std::string::npos) 
    {
        Body = Body.substr(0, Body.find(Boundary));
        this->DataFinish = true;
    }
    if (this->fd > 0) 
    {
        if (Body.empty())
            return ;
        write(this->fd, Body.c_str(), Body.size());
    }
}

void    FileUpload::ParseBody(std::string Body, std::string Boundary, std::string path) {
    if (this->DataFinish)
        return ;
    this->pos = Body.find(Boundary);
    if (this->pos == 0) {
        this->pos = Body.find(this->ContentDisposition);
        if (this->pos != std::string::npos)
            this->ParseContentDisposition(Body);
        this->pos = Body.find(this->ContentType);
        if (this->pos != std::string::npos)
            this->ParseContentType(Body);
    }
    this->OpenFile(path);
    this->WriteToFile(Body, Boundary);
}
