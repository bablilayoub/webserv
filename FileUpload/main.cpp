/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:38:32 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/12 17:49:35 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"

int main() {
    std::string FirstChunk = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"firstChunk\"; filename=\"firstChunk.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the first chunk\r\n"
    "data for the first chunk\r\n"
    "data for the first chunk\r\n"
    "data for the first chunk\r\n"
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"firsta\"; filename=\"firsta.txt\"\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n";
    
    std::string SecondeChunk = 
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n";    
    
    std::string ThirdChunk = 
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "data for the firsta name\r\n"
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"third\"; filename=\"third.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n";

    std::string LastChunk = 
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunk\r\n"
    "data for the third chunkhello----------------------------671379837443287244198638--\r"
    "Some Other Data\r\n"
    "Some Other Data\r\n"
    "Some Other Data\r\n"
    "Some Other Data\r\n"
    "Some Other Data\r\n"
    "Some Other Data\r\n";


    std::string MiddleChunk = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"Middle\"; filename=\"Middle.txt\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the Middle image\r\n";

    // "Content-Disposition: form-data; name=\"third\"; filename=\"third.txt\"\r\n"
    // "Content-Type: image/png\r\n"
    // "data for the third chunk\r\n"
    // "data for the third chunk\r\n"
    // "data for the third chunk\r\n"
    // "data for the third chunk\r\n";

    std::map<int, FileUpload> DataMap;

    DataMap[1].ParseBody(FirstChunk);
    DataMap[1].ParseBody(SecondeChunk);
    DataMap[1].ParseBody(ThirdChunk);
    DataMap[1].ParseBody(LastChunk);
    // DataMap[2].ParseBody(MiddleChunk);
}