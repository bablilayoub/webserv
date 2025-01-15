/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Global.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 18:46:49 by abablil           #+#    #+#             */
/*   Updated: 2025/01/15 16:02:41 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <exception>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stack>
#include <sys/stat.h>
#include <dirent.h>

#include "../FileUpload/FileUpload.hpp"

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"

#define BOUNDARY_PREFIX "boundary="
#define CONTENT_LENGTH_PREFIX "Content-Length: "
#define HOST_PREFIX "Host: "