/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:38:32 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/12 13:13:30 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

int main() {
    std::string FirstChunk = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"test\"; filename=\"test.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the image\r\n"
    "data for the image\r\n";
    
    std::string SecondeChunk = 
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n"
    "Another Data for the image\r\n";
    
    std::string ThirdChunk = 
    "data for the image\r\n"
    "data for the image\r\n"
    "data for the image\r\n"
    "data for the image\r\n"
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"third\"; filename=\"third.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "data for the image\r\n"
    "data for the image\r\n"
    "data for the image\r\n"
    "data for the image\r\n";
    
    std::string MiddleChunk = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"Middle\"; filename=\"Middle.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the Middle image\r\n";

    std::string LastChunk = 
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "data for the Middle image\r\n"
    "----------------------------671379837443287244198638--\r";

    std::string TestChunk = 
    "data for the Middle image----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"chunk\"; filename=\"chunk.txt\"\r\n";

    FileUpload InsFileUpload;
    std::map<int, FileUpload> DataMap;

    DataMap[1].ParseBody(FirstChunk);
    // DataMap[1].ParseBody(ThirdChunk);
    // DataMap[2].ParseBody(MiddleChunk);
    // DataMap[1].ParseBody(LastChunk);
    // DataMap[1].ParseBody(SecondeChunk);
    // DataMap[1].ParseBody(TestChunk);

}