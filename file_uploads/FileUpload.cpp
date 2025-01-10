/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:34:45 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/10 17:20:47 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

FileUpload::FileUpload() {
    this->BoundaryString = "--------------------------671379837443287244198638";
    this->Body = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"aimen\"\r\n"
    "\r\n"
    "Redx\r\n"
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"oussama\"\r\n"
    "\r\n"
    "splinta\r\n"
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"test\"; filename=\"Screen Shot 2025-01-09 at 7.58.19 PM.png\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the image\r\n"
    "----------------------------671379837443287244198638--\r";
}

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