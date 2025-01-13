// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   main.cpp                                           :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2025/01/09 14:38:32 by aitaouss          #+#    #+#             */
// /*   Updated: 2025/01/13 11:51:14 by aitaouss         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #include "FileUpload.hpp"

// int main() {
//     std::string FirstChunk = 
//     "----------------------------671379837443287244198638\r\n"
//     "Content-Disposition: form-data; name=\"firstChunk\"\r\n"
//     "Content-Type: image/png\r\n"
//     "\r\n"
//     "data for the first chunk\r\n"
//     "data for the first chunk\r\n"
//     "data for the first chunk\r\n"
//     "data for the first chunk\r\n";
//     // "----------------------------671379837443287244198638\r\n"
//     // "Content-Disposition: form-data; name=\"firsta\"; filename=\"firsta.txt\"\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n"
//     // "data for the firsta name\r\n";
    
//     std::string SecondeChunk = 
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n";    
    
//     std::string ThirdChunk = 
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "data for the firsta name\r\n"
//     "----------------------------671379837443287244198638\r\n"
//     "Content-Disposition: form-data; name=\"third\"; filename=\"third.txt\"\r\n"
//     "Content-Type: image/png\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n";

//     std::string LastChunk = 
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunk\r\n"
//     "data for the third chunkhello----------------------------671379837443287244198638--\r"
//     "Some Other Data\r\n"
//     "Some Other Data\r\n"
//     "Some Other Data\r\n"
//     "Some Other Data\r\n"
//     "Some Other Data\r\n"
//     "Some Other Data\r\n";


//     std::string SecondeFd = 
//     "----------------------------671379837443287244198638\r\n"
//     "Content-Disposition: form-data; name=\"SecondeFd\"; filename=\"SecondeFd.txt\"\r\n"
//     "Content-Type: image/png\r\n"
//     "\r\n"
//     "data for the SecondeFd image\r\n";

//     std::string SecondeFdNext = 
//     "data for the SecondeFd image\r\n"
//     "data for the SecondeFd image\r\n"
//     "data for the SecondeFd image\r\n"
//     "data for the SecondeFd image\r\n"
//     "----------------------------671379837443287244198638\r\n"
//     "Content-Disposition: form-data; name=\"SecondeNext\"; filename=\"SecondeNext.txt\"\r\n"
//     "Content-Type: image/png\r\n"
//     "data for the SecondeNext image\r\n"
//     "data for the SecondeNext image\r\n"
//     "data for the SecondeNext image\r\n"
//     "data for the SecondeNext image\r\n";

//     // "Content-Disposition: form-data; name=\"third\"; filename=\"third.txt\"\r\n"
//     // "Content-Type: image/png\r\n"
//     // "data for the third chunk\r\n"
//     // "data for the third chunk\r\n"
//     // "data for the third chunk\r\n"
//     // "data for the third chunk\r\n";

//     std::map<int, FileUpload> DataMap;

//     DataMap[1].ParseBody(FirstChunk);
//     // DataMap[2].ParseBody(SecondeFd);
//     // DataMap[2].ParseBody(SecondeFdNext);
//     // DataMap[1].ParseBody(SecondeChunk);
//     // DataMap[1].ParseBody(ThirdChunk);
//     // DataMap[1].ParseBody(LastChunk);
// }