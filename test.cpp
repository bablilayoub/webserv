/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 10:04:17 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/14 10:07:21 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sstream>


int main() {
    std::string data = 
    "test Lines for the new line 1\n"
    "test Lines for the new line 2\n"
    "test Lines for the new line 3\n"
    "test Lines for the new line 4\n"
    "test Lines for the new line 5\n";

    std::stringstream ss;
    std::string line;
    ss << data;
    while (std::getline(ss, line)) {
        if (line.find("3") != std::string::npos) {
            std::getline(ss, line);
        }
        std::cout << line << std::endl;
    }
}