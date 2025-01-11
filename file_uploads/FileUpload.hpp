/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/11 15:44:52 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

// Containers
#include <vector>
#include <map>
#include <algorithm>

#define NAME_LENGHT 5
#define FILE_NAME_LENGHT 9
#define CONTENT_DISPOSITION_LENGHT 38

// Data struct
class   BodyData {
    public:
        BodyData();
        ~BodyData();
        int fd;
        std::string Name;
        std::string ContentType;
        std::string Data;
        std::string filename;
};

class   FileUpload {
    private:
        int DataFinish;
    public:
        int fd;
        FileUpload();
        ~FileUpload();
        int FirstTime;
        std::string BoundaryString;
        BodyData DataBody;
        void    ParseBody(std::string Body);
};
