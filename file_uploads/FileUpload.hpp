/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUpload.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:33:18 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/10 17:52:38 by aitaouss         ###   ########.fr       */
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

class   FileUpload {
    public:
        std::vector<std::string> parts;
        FileUpload();
        std::string Body;
        std::string BoundaryString;
        std::vector<std::string> SplitBody(std::string str, std::string delimiter);
};