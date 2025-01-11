/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 11:35:19 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string.h>
#include <fstream>

// Containers
#include <vector>
#include <map>
#include <algorithm>

#define NAME_LENGHT 5
#define FILE_NAME_LENGHT 9
#define CONTENT_DISPOSITION_LENGHT 38

class   FileUpload {
    public:
        std::vector<std::string> parts;
        FileUpload();
        std::string Body;
        std::string BoundaryString;
        std::vector<std::string> SplitBody(std::string str, std::string delimiter);
        void    ParseBody(std::string Body);
        ~FileUpload();
};

// Data struct
class   BodyData {
    public:
        BodyData();
        std::string Name;
        std::string ContentType;
        std::string Data;
        std::string filename;
        ~BodyData();
};
