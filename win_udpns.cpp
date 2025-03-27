#include "win_udpns.h"
#include <cstddef>
#include <iostream>
#include <string_view>
namespace UDPNS
{

    const int GAI_OK{ 0 };
    const int BIND_ERROR{ -1 };
    const int RECV_FORM_FLAG{ 0 };
    const int SENDTO_FLAG{ 0 };
    const int RECVFORM_FLAG{ 0 };
    const std::string_view MY_PORT{ "4951" };
    const size_t BUF_LEN{ 100 };
    const int ADDR_LEN{ 18 };

    void* get_in_addr(struct sockaddr* sa)
    {
        if (sa->sa_family == AF_INET) {
            return &((reinterpret_cast<struct sockaddr_in*>(sa))->sin_addr);
        }
        return &((reinterpret_cast<struct sockaddr_in6*>(sa))->sin6_addr);
    }

    bool initWSA()
    {
        WSADATA wsaData;
        if (int rc{ WSAStartup(MAKEWORD(2, 2), &wsaData) }; rc != 0) {
            std::cerr << "WSAStartup failed " << rc << '\n';
            return false;
        }
        // std::cout << "VERSION: " << static_cast<int>(LOBYTE(wsaData.wVersion)) << " | " << static_cast<int>(HIBYTE(wsaData.wVersion)) << '\n';
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
            std::cerr << "Version 2.2 of Winsock not available.\n";
            WSACleanup();
            return false;
        }
        return true;
    }

    bool UDP::makeBuffers()
    {
        buf = new char[BUF_LEN];
        if (buf == nullptr) {
            return false;
        }
        return true;
    }

    void UDP::freeResults()
    {
        if (results) {
            freeaddrinfo(results);
            results = nullptr;
        }
    }

    bool UDP::gai(std::string_view host, std::string_view port)
    {
        char* name{ nullptr };
        char* serv{ nullptr };
        struct addrinfo req {};
        req.ai_family = AF_INET;    // IPv4
        req.ai_socktype = SOCK_DGRAM; // datagram (UDP)

        if (host.empty()) {
            req.ai_flags = AI_PASSIVE; // use my IP
        }
        else {
            name = const_cast<char*>(host.data());
        }

        if (port.empty()) {
            serv = const_cast<char*>(MY_PORT.data());
        }
        else {
            serv = const_cast<char*>(port.data());
        }

        freeResults(); // check and free <addrinfo* results>

        if (auto rv{ getaddrinfo(name, serv, &req, &results) }; rv != GAI_OK) {
            std::cerr << "getaddrinfo: " << rv << '\n';
            return false;
        }

        return true;
    }

    bool UDP::createSocket(std::string_view host, std::string_view port, bool RX)
    {

        if (rx != INVALID_SOCKET) {
            std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
            return false;
        }

        if (tx != INVALID_SOCKET) {
            std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
            return false;
        }

        if (!gai(host, port)) {
            std::cerr << "GAI DEAD\n";
            return false;
        }

        SOCKET sockfd{ INVALID_SOCKET };
        for (target = results; target; target = target->ai_next) {
            if (sockfd = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
                sockfd == INVALID_SOCKET) {
                std::cerr << "socket miss\n";
                continue;
            }
            if (RX) {
                if (bind(sockfd, target->ai_addr, static_cast<int>(target->ai_addrlen)) == SOCKET_ERROR) {
                    closesocket(sockfd);
                    std::cerr << "bind miss\n";
                    continue;
                }
            }
            break; // this one is good so leave without setting to next addrinfo...
        }

        if (target == nullptr) {
            std::cerr << "failed to get socket/bind\n";
            return false;
        }

        if (RX) {
            rx = sockfd;
        }
        else {
            tx = sockfd;
        }
        return true; // all good
    }

    bool UDP::createRX(std::string_view host, std::string_view port)
    {
        return createSocket(host, port, true);
    }

    bool UDP::createTX(std::string_view host, std::string_view port)
    {
        return createSocket(host, port, false);
    }


    bool UDP::txAllocated()
    {
        return !(tx == INVALID_SOCKET);
    }

    bool UDP::rxAllocated()
    {
        return !(rx == INVALID_SOCKET);
    }

    bool UDP::receive()
    {
        if (!rxAllocated()) {
            std::cerr << "RX NOT ALLOCATED YET TRYING RECEIVE!\n";
            return false;
        }
        if (rx_bytes = recv(rx, buf, BUF_LEN - 1, UDPNS::RECVFORM_FLAG); rx_bytes == -1) {
            std::cerr << "recvform error\n";
            return false;
        }
        buf[rx_bytes] = '\0';
        return true;
    }

    bool UDP::transmit(std::string_view msg)
    {
        if (tx_bytes = sendto(tx, msg.data(), static_cast<int>(msg.length()), UDPNS::SENDTO_FLAG,
            target->ai_addr, static_cast<int>(target->ai_addrlen));
            tx_bytes == -1) {
            std::cerr << "send error\n";
            return false;
        }
        std::cout << "send successful\n";
        return true;
    }

    void UDP::clearAll()
    {
        if (rx != INVALID_SOCKET) {
            closesocket(rx);
            rx = INVALID_SOCKET;
        }
        if (tx != INVALID_SOCKET) {
            closesocket(tx);
            tx = INVALID_SOCKET;
        }
        if (buf) {
            delete[] buf;
            buf = nullptr;
        }
        freeResults();
        WSACleanup();
    }

    UDP::~UDP()
    {
        clearAll();
    }

    




} // namespace UDPNS
