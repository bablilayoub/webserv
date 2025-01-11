/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 11:55:28 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->BoundaryString = "--------------------------671379837443287244198638";
    this->Body = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"test\"; filename=\"Screen Shot 2025-01-09 at 7.58.19 PM.png\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the image\r\n"
    "----------------------------671379837443287244198638--\r";
    // this->Body = 
    // "----------------------------671379837443287244198638\r\n"
    // "Content-Disposition: form-data; name=\"aimen\"\r\n"
    // "\r\n"
    // "Redx\r\n"
    // "----------------------------671379837443287244198638\r\n"
    // "Content-Disposition: form-data; name=\"oussama\"\r\n"
    // "\r\n"
    // "splinta\r\n"
    // "----------------------------671379837443287244198638\r\n"
    // "Content-Disposition: form-data; name=\"test\"; filename=\"Screen Shot 2025-01-09 at 7.58.19 PM.png\"\r\n"
    // "Content-Type: image/png\r\n"
    // "\r\n"
    // "data for the image\r\n"
    // "----------------------------671379837443287244198638--\r";
}

FileUpload::~FileUpload() {
    
}

// not use for now
std::vector<std::string> FileUpload::SplitBody(std::string str, std::string delimiter) {
    std::string boundary = "--" + delimiter;
    std::vector<std::string> parts;

    size_t pos = str.find(boundary);
    while (pos != std::string::npos) {
        str = str.substr(pos + boundary.length());
        pos = str.find(boundary);
        parts.push_back(str.substr(0, pos));
    }
    parts.erase(parts.end() - 1);
    return parts;
}

// Not use for now
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
    size_t pos = Body.find("Content-Disposition: form-data; name=\"");

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
            // data
            // open file and check it if is open correctly
            // if (FileName.c_str()) {
            //     std::ofstream FileCreate(FileName);
            //     if (!FileCreate.is_open()) {
            //         std::cerr << "Error: Could not open the file: " << FileName << std::endl;
            //         return ;
            //     }
            //     FileCreate.close();
            // }
        }
    }    
}