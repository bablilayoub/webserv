/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:38:32 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 12:08:31 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"



// int main() {
//     FileUpload ins;

//     ins.parts = ins.SplitBody(ins.Body, ins.BoundaryString);
//     // print the vector
//     // for (size_t i = 0; i < ins.parts.size(); i++) {
//     //     std::cout << "--------" << i << "--------";
//     //     std::cout << ins.parts[i];
//     // }

//     std::vector<BodyData> bodySegments;

//     for (size_t i = 0; i < ins.parts.size(); i++) {
//         std::string part = ins.parts[i];
//         size_t pos = part.find(NAME);
//         std::string str = part.substr(pos + NAME_LENGHT, part.length());
        // for (size_t i = 1; i < str.size(); i++) {
        //     if (str[i] == '\"') {
        //         pos = i;
        //         break;
        //     }
        // }
//         str = str.substr(1, pos - 1);
//         BodyData data;

//         data.Name = str;
//         bodySegments.push_back(data);
//     }

//     for (size_t i = 0; i < bodySegments.size(); i++) {   
//         std::cout << "Name = " << bodySegments[i].Name << std::endl;
//         std::cout << "Data = " << bodySegments[i].Data << std::endl;
//         std::cout << "ContentType = " << bodySegments[i].ContentType << std::endl;
//         std::cout << "filename = " << bodySegments[i].filename << std::endl << std::endl;
//     }
//     return 0;
// }

int main() {
    std::string Body = 
    "----------------------------671379837443287244198638\r\n"
    "Content-Disposition: form-data; name=\"test\"; filename=\"Screen Shot 2025-01-09 at 7.58.19 PM.png\"\r\n"
    "Content-Type: image/png\r\n"
    "\r\n"
    "data for the image\r\n"
    "----------------------------671379837443287244198638--\r";

    FileUpload ins;

    ins.ParseBody(Body);
}