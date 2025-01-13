/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/13 17:23:00 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->BoundaryString = "----------------------------671379837443287244198638";
    this->HeaderFetched = false;
    this->FetchData = true;
    this->fd = -42;
    this->Name = "";
    this->FileName = "";
    this->MimeType = "";
    this->Data = "";
    this->DataFinish = false;
}

FileUpload::~FileUpload() {
    
}

// void    FileUpload::ParseBody(std::string Body) {
//     std::string name;
//     std::string FileName;
//     std::string Contenttype;
//     std::string BodySafe = Body;
//     size_t pos = Body.find("Content-Disposition: form-data; name=\"");

//     if (pos != std::string::npos) {
//         this->FetchData = true;
//     }

//     if (this->FetchData) {
//         this->FetchData = false;
//         // to extract data
//         if (pos != std::string::npos) {
//             // extract Name
//             Body = Body.substr(pos + CONTENT_DISPOSITION_LENGHT, Body.length());
//             for (size_t i = 0; i < Body.size(); i++) {
//                 if (Body[i] == '\"') {
//                     pos = i;
//                     break;
//                 }
//             }
//             name = Body.substr(0, pos);
//             // extract Filename
//             pos = Body.find("filename");
//             Body = Body.substr(pos + FILE_NAME_LENGHT, Body.length());
//             for (size_t i = 1; i < Body.size(); i++) {
//                 if (Body[i] == '\"') {
//                     pos = i; 
//                     break;
//                 }
//             }
//             FileName = Body.substr(1, pos - 1);
//             // To extract content type
//             if (FileName.c_str()) {
//                 pos = Body.find("Content-Type");
//                 if (pos != std::string::npos) {
//                     Body = Body.substr(pos + 14, Body.length());
//                     for (size_t i = 0; i < Body.size(); i++) {
//                         if (Body[i] == '\n') {
//                             pos = i;
//                             break;
//                         }
//                     }
//                     Contenttype = Body.substr(0, pos - 1);
//                 }
//             }
//         }
//         std::cout << "------- Infos -------" << std::endl;
//         std::cout << name << std::endl;
//         std::cout << FileName << std::endl;
//         std::cout << Contenttype << std::endl;
//     }
    
//     pos = BodySafe.find("Content-Type");

//     if (pos == std::string::npos)
//         pos = BodySafe.find("Content-Disposition");

//     // // open the filename if exist
//     if (FileName.length() != 0) {
//         this->fd = open(FileName.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
//         if (this->fd < 0) {
//             std::cout << "Failed to open the file : " << FileName << std::endl;
//             return ;
//         }
//     }

//     if (pos != std::string::npos) {
//         BodySafe = BodySafe.substr(pos, BodySafe.length());
//         pos = BodySafe.find("\n");
//         if (pos == (BodySafe.length() - 1)) {
//             return ;
//         }
//         if (pos != std::string::npos) {
//             pos = pos + 3;
//             const char *str = BodySafe.c_str();
//             int LastPos = pos;
//             while (str[pos]) {
//                 pos++;
//             }
//             BodySafe = BodySafe.substr(LastPos, pos);
//         }
//     }

//     if (BodySafe.find(this->BoundaryString + "--") != std::string::npos) {
//         this->DataFinish = true;
//         std::cout << "Body Finish" << std::endl;
//     }
    
//     // // if data == npos so the data of the file or the text is the BodySafe and there is no boundary string
//     pos = BodySafe.find(this->BoundaryString);
//     std::string BodyData = BodySafe;
//     if (pos != std::string::npos)
//         BodyData = BodySafe.substr(0, pos);

//     std::cout << "-------- Data " << name << "-------" << std::endl;
//     std::cout << BodyData;
//     std::cout << "-------- Data End -------" << std::endl;
//     if (this->fd > 0) {
//         write(this->fd, BodyData.c_str(), BodyData.length());
//     }
//     // // to do now check the boundary if there is
// }

void    FileUpload::ParseBody(std::string Body, std::string Boundary) {
    // std::stringstream ss;
    std::stringstream ss(Body);
    std::string line;
    std::string substr;
    size_t pos;
    std::string Data;
    pos = Boundary.find("\r");
    std::string LastBoundary = Boundary.substr(0, pos) + "--";
    // ss << Body;

    while (std::getline(ss, line) && !this->DataFinish) {
        // if Boundary-- then Break the loop
        if (line.find(LastBoundary) != std::string::npos) {
            this->DataFinish = true;
            std::cout << "Data Finish" << std::endl;
            return ;
        }
        // //Get the name and the filename if exist
        pos = line.find("Content-Disposition: form-data;");
        if (pos != std::string::npos) {
            // close(this->fd);
            // this->fd = -42;
            pos = line.find("name=\"");
            substr = line.substr(pos + NAME_LENGHT + 1, line.length());
            pos = substr.find("\"");
            this->Name = substr.substr(0, pos);
            pos = substr.find("filename=\"");
            if (pos != std::string::npos) {
                substr = substr.substr(pos + FILE_NAME_LENGHT + 1 , substr.length());
                pos = substr.find("\"");
                this->FileName = substr.substr(0, pos);
            }
            else {
                this->FileName = "";
            }
            this->HeaderFetched = true;
        }

        // Get the Mime type
        pos = line.find("Content-Type:");
        if (pos != std::string::npos) {
            substr = line.substr(pos + 14, line.length());
            pos = substr.find("/");
            this->MimeType = substr.substr(pos + 1, substr.find("\r") - (pos + 1));
            if (this->FileName.empty()) {
                this->FileName = this->Name + "." + this->MimeType;
            }
            if (!this->FileName.empty()) {
                this->fd = open(this->FileName.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
                std::cout << "file Opened" << std::endl;
                if (this->fd < 0) {
                    std::cout << "Failed to open the file : " << this->FileName << std::endl;
                    return ;
                }
            }
        }

        // check if there is in the line Dipsosition or type or Boundary if There is Boundary 
        // print the data untill the pos of the first pos
        pos = line.find(CONTENT_DISPOSITION);
        if (pos == std::string::npos && line.length() > 0) {
            pos = line.find(CONTENT_TYPE);
            if (pos == std::string::npos) {
                this->FileName = "";
                pos = line.find(Boundary);
                if (pos != 0) {
                    if (pos != std::string::npos) {
                        if (pos != 0) {
                            for (size_t i = 0; i < line.length(); i++) {
                                if (i == pos)
                                    break;
                            }
                            Data = line.substr(0, pos);
                            Data += "\n";
                            if (this->fd > 0) {
                                write(fd, Data.c_str(), Data.length());
                            }
                        }
                    }
                    else {
                        Data = line;
                        Data += "\n";
                        if (this->fd > 0) {
                            write(fd, Data.c_str(), Data.length());
                        }
                    }
                }
            }
        }
    }

    
    // std::cout << "--------- Data Info ---------" << std::endl;
    // std::cout << this->Name << std::endl; 
    // std::cout << this->FileName << std::endl;
    // std::cout <<  this->MimeType << std::endl;
    // std::cout << "--------- Data Info Ended ---------" << std::endl;
}
