/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/15 17:26:56 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/15 17:37:10 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

std::string generate_random_string(int length) {
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string newstr;
    for (int i = 0; i < length; i++) {
        newstr += str[rand() % str.size()];
    }
    return newstr;
}

int main() {
    // Seed the random number generator with the current time
    std::srand(std::time(0));

    std::string str = generate_random_string(10);
    std::cout << str << std::endl;
    return 0;
}