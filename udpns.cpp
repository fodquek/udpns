#include "udpns.h"
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <netdb.h>
#include <string_view>
#include <sys/socket.h>
namespace UDPNS
{

const int GAI_OK{ 0 };
const int SOCKET_ERROR{ -1 };
const int BIND_ERROR{ -1 };
const int RECV_FORM_FLAG{ 0 };
const int SENDTO_FLAG{ 0 };
const int RECVFORM_FLAG{ 0 };
const std::string_view MY_PORT{ "4951" };
const size_t BUF_LEN{ 100 };
const int ADDR_LEN{ 18 };
// const int FCNTL_ERR {-1 };

void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &((reinterpret_cast<struct sockaddr_in*>(sa))->sin_addr);
    }
    return &((reinterpret_cast<struct sockaddr_in6*>(sa))->sin6_addr);
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
    struct addrinfo req{};
    req.ai_family   = AF_INET;    // IPv4
    req.ai_socktype = SOCK_DGRAM; // datagram (UDP)

    if (host.empty()) {
        req.ai_flags = AI_PASSIVE; // use my IP
    } else {
        name = const_cast<char*>(host.data());
    }

    if (port.empty()) {
        serv = const_cast<char*>(MY_PORT.data());
    } else {
        serv = const_cast<char*>(port.data());
    }

    freeResults(); // check and free <addrinfo* results>

    if (auto rv{ getaddrinfo(name, serv, &req, &results) }; rv != GAI_OK) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << '\n';
        return false;
    }

    return true;
}

bool UDP::createSocket(std::string_view host, std::string_view port, bool RX)
{

    if (rx != SOCKET_ERROR) {
        std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
        return false;
    }

    if (tx != SOCKET_ERROR) {
        std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
        return false;
    }

    if (!gai(host, port)) {
        std::cerr << "GAI DEAD\n";
        return false;
    }

    int sockfd;
    for (target = results; target; target = target->ai_next) {
        if (sockfd = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
            sockfd == SOCKET_ERROR) {
            std::cerr << "socket miss\n";
            continue;
        }
        if (RX) {
            if (bind(sockfd, target->ai_addr, target->ai_addrlen) == BIND_ERROR) {
                close(sockfd);
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

    // if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == FCNTL_ERR) {
    //     if (EAGAIN == errno) {
    //         std::cerr << "EAGAIN errno fcntl!!\n";
    //         return false;
    //     } else if (EWOULDBLOCK == errno) {
    //         std::cerr << "EWOULDBLOCK errno fcntl!!\n";
    //         return false;
    //     }
    // }
    
    if (RX) {
        rx = sockfd;
    } else {
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
    return !(tx == SOCKET_ERROR);
}

bool UDP::rxAllocated()
{
    return !(rx == SOCKET_ERROR);
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
    if (tx_bytes = sendto(tx, msg.data(), msg.length(), UDPNS::SENDTO_FLAG,
                          target->ai_addr, target->ai_addrlen);
        tx_bytes == -1) {
        std::cerr << "send error\n";
        return false;
    }
    std::cout << "send successful\n";
    return true;
}

void UDP::clearAll()
{
    if (rx != SOCKET_ERROR) {
        close(rx);
        rx = SOCKET_ERROR;
    }
    if (tx != SOCKET_ERROR) {
        close(tx);
        tx = SOCKET_ERROR;
    }
    if (buf) {
        delete[] buf;
        buf = nullptr;
    }
    freeResults();
}

UDP::~UDP()
{
    clearAll();
}

} // namespace UDPNS
