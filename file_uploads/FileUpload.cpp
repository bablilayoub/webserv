/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 15:49:28 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->BoundaryString = "----------------------------671379837443287244198638";
    this->DataFinish = false;
    this->FirstTime = true;
    this->fd = -42;
}

FileUpload::~FileUpload() {
    
}

BodyData::BodyData() {
    this->Data = "";
    this->ContentType = "";
    this->Name = "";
    this->filename = "";
}

BodyData::~BodyData() {
}

void    FileUpload::ParseBody(std::string Body) {
    std::string name;
    std::string FileName;
    std::string Contenttype;
    std::string BodySafe = Body;
    size_t pos = Body.find("Content-Disposition: form-data; name=\"");
    if (pos != std::string::npos) {
        this->FirstTime = true;
    }
    if (this->FirstTime) {
        this->FirstTime = false;
        // to extract data
        if (pos != std::string::npos) {
            // extract Name
            Body = Body.substr(pos + CONTENT_DISPOSITION_LENGHT, Body.length());
            for (size_t i = 0; i < Body.size(); i++) {
                if (Body[i] == '\"') {
                    pos = i;
                    break;
                }
            }
            name = Body.substr(0, pos);
            // extract Filename
            pos = Body.find("filename");
            Body = Body.substr(pos + FILE_NAME_LENGHT, Body.length());
            for (size_t i = 1; i < Body.size(); i++) {
                if (Body[i] == '\"') {
                    pos = i; 
                    break;
                }
            }
            FileName = Body.substr(1, pos - 1);
            // To extract content type
            if (FileName.c_str()) {
                pos = Body.find("Content-Type");
                if (pos != std::string::npos) {
                    Body = Body.substr(pos + 14, Body.length());
                    for (size_t i = 0; i < Body.size(); i++) {
                        if (Body[i] == '\n') {
                            pos = i;
                            break;
                        }
                    }
                    Contenttype = Body.substr(0, pos - 1);
                }
                std::cout << "name : " << name << std::endl;
                std::cout << "file name : " << FileName << std::endl;
                std::cout << "MIME : " << Contenttype << std::endl;
            }
        }
    }
    if (this->fd == -42) {
        this->fd = open(FileName.c_str(), O_RDWR | O_CREAT);
        if (this->fd > 0) {
            // write the data for the safeBody in the fd
            write(this->fd, BodySafe.c_str(), BodySafe.length());
        }   
    }
    size_t posBoundary = BodySafe.find(this->BoundaryString);

}