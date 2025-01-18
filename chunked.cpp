/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/17 09:06:51 by aitaouss          #+#    #+#             */
/*   Updated: 2025/01/18 18:14:03 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Server/TcpServer.hpp"
#include "./Client/Config.hpp"

void eraseBody(std::string &Body, std::string StringtoErase) {   
    size_t pos;
    std::string ToJoin = "";
    pos = Body.find(StringtoErase);
    if (pos != std::string::npos) {
        ToJoin = Body.substr(0, pos);
        Body = Body.substr(pos + StringtoErase.length() + 2);
        Body = ToJoin + Body;
    }
}

// int main() {
//     std::string requestChunked = 
//     "POST /chunked HTTP/1.1\r\n"
//     "Host: localhost:9000\r\n"
//     "User-Agent: curl/7.68.0\r\n"
//     "Accept: */*\r\n"
//     "Transfer-Encoding: chunked\r\n"
//     "\r\n"
//     "b\r\n"
//     "hello world\r\n"
//     "0\r\n"
//     "\r\n";

//     size_t pos;
//     std::string ChunkedString = "Transfer-Encoding: chunked";
//     bool isChunked = false;

//     if ((pos = requestChunked.find("Transfer-Encoding: chunked")) != std::string::npos) {
//         isChunked = true;
//     }

//     if (isChunked) {    
//         requestChunked = requestChunked.substr(pos + ChunkedString.length(), requestChunked.length());
//         std::cout << requestChunked << std::endl;
//     }
// }

// int main() {
    std::string data = "Hello World for the first Time im here 10000rncan u open this";
//     std::string data2 = "10000rnHello World for the first Time im here can u open this";
//     std::string toerase = "10000";

//     eraseBody(data2, toerase);
//     std::cout << data2 << std::endl;
// }

int main() {
    std::string data = "Hello World for the first Time im here 10000rncan u open this";
    
    std::string substr = data.substr(0, 5600);
    std::cout << substr << std::endl;
}