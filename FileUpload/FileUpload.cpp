/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 20:09:30 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->BoundaryString = "----------------------------671379837443287244198638";
    this->DataFinish = false;
    this->FirstTime = true;
    this->fd = 1337;
}

FileUpload::~FileUpload() {
    
}

// BodyData::BodyData() {
//     this->Data = "";
//     this->ContentType = "";
//     this->Name = "";
//     this->filename = "";
// }

// BodyData::~BodyData() {
// }

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
            }
        }
    }
    
    std::stringstream ss;
    ss << BodySafe;
    std::string line;
    
    pos = BodySafe.find("Content-Type");

    if ((pos = BodySafe.find("Content-Type")) == std::string::npos)
        pos = BodySafe.find("Content-Disposition");

    // open the filename if exist
    if (FileName.length() != 0) {
        if (this->fd == 1337) {
            this->fd = open(FileName.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
            if (this->fd < 0) {
                std::cout << "Failed to open the file : " << FileName << std::endl;
                return ;
            }
        }
    }

    if (pos != std::string::npos) {
        BodySafe = BodySafe.substr(pos, BodySafe.length());
        pos = BodySafe.find("\n");
        if (pos != std::string::npos) {
            pos = pos + 3;
            const char *str = BodySafe.c_str();
            int LastPos = pos;
            while (str[pos]) {
                pos++;
            }
            BodySafe = BodySafe.substr(LastPos, pos);
        }
    }
    // if data == npos so the data of the file or the text is the BodySafe and there is no boundary string
    // if (pos == std::string::npos && BodySafe.find(this->BoundaryString) == std::string::npos) {
        if (fd > 0) {
            write(this->fd, BodySafe.c_str(), BodySafe.length());
        }
    // }

    // to do now check the boundary if there is
}