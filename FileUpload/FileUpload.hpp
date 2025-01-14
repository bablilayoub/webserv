/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/14 09:33:55 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

#include <vector>
#include <map>
#include <algorithm>

#define NAME_LENGHT 5
#define FILE_NAME_LENGHT 9
#define CONTENT_DISPOSITION_LENGHT 38
#define CONTENT_DISPOSITION "Content-Disposition"
#define CONTENT_TYPE "Content-Type"

class   FileUpload {
    private:
        int DataFinish;
        int FetchData;
        std::string CurrentFileName;
        std::string Name;
        std::string FileName;
        std::string MimeType;
        int HeaderFetched;
        std::string Data;
    public:
        int fd;
        FileUpload();
        ~FileUpload();
        std::string BoundaryString;
        void    ParseBody(std::string Body, std::string Boundary);
        
};
