/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:38:32 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/10 17:56:22 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUpload.hpp"



int main() {
    FileUpload ins;

    ins.parts = ins.SplitBody(ins.Body, ins.BoundaryString);
    for (size_t i = 0; i < ins.parts.size(); i++) {
        std::cout << "--------" << i << "--------";
        std::cout << ins.parts[i];
    }
    return 0;
}