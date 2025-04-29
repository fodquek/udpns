#include "udpns.h"
#include <iostream>

namespace UDPNS
{

    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET)
        {
            return &((reinterpret_cast<struct sockaddr_in *>(sa))->sin_addr);
        }
        return &((reinterpret_cast<struct sockaddr_in6 *>(sa))->sin6_addr);
    }

#ifdef UDPNS_WINDOWS
    bool initWSA()
    {
        WSADATA wsaData;
        if (int rc{WSAStartup(MAKEWORD(2, 2), &wsaData)}; rc != 0)
        {
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
#endif

    bool UDP::makeBuffers()
    {
        buf = new char[BUF_LEN];
        if (buf == nullptr)
        {
            return false;
        }
        return true;
    }

    void UDP::freeResults()
    {
        if (results)
        {
            freeaddrinfo(results);
            results = nullptr;
        }
    }

    bool UDP::gai(std::string_view host, std::string_view port)
    {
        char *name{nullptr};
        char *serv{nullptr};
        struct addrinfo req{};
        req.ai_family = AF_INET;      // IPv4
        req.ai_socktype = SOCK_DGRAM; // datagram (UDP)

        if (host.empty())
        {
            req.ai_flags = AI_PASSIVE; // use my IP
        }
        else
        {
            name = const_cast<char *>(host.data());
        }

        if (port.empty())
        {
            serv = const_cast<char *>(MY_PORT.data());
        }
        else
        {
            serv = const_cast<char *>(port.data());
        }

        freeResults(); // check and free <addrinfo* results>

        if (auto rv{getaddrinfo(name, serv, &req, &results)}; rv != GAI_OK)
        {
            std::cerr << "getaddrinfo: " << gai_strerror(rv) << '\n';
            return false;
        }

        return true;
    }

    bool UDP::createSocket(std::string_view host, std::string_view port, bool RX)
    {

        if (rx != BAD_SOCKET)
        {
            std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
            return false;
        }

        if (tx != BAD_SOCKET)
        {
            std::cerr << "RX SOCKET ALIVE YET CREATING NEW ONE?!";
            return false;
        }

        if (!gai(host, port))
        {
            std::cerr << "GAI DEAD\n";
            return false;
        }

        auto sockfd{BAD_SOCKET};
        for (target = results; target; target = target->ai_next)
        {
            if (sockfd = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
                sockfd == BAD_SOCKET)
            {
                std::cerr << "socket miss\n";
                continue;
            }
            if (RX)
            {
                if (bind(sockfd, target->ai_addr, static_cast<ai_addrlen_t>(target->ai_addrlen)) == BAD_BIND)
                {
#ifdef UDPNS_WINDOWS
                    closesocket(sockfd);
#else
                    close(sockfd);
#endif
                    std::cerr << "bind miss\n";
                    continue;
                }
            }
            break; // this one is good so leave without setting to next addrinfo...
        }

        if (target == nullptr)
        {
            std::cerr << "failed to get socket/bind\n";
            return false;
        }

        if (RX)
        {
            rx = sockfd;
        }
        else
        {
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
        return !(tx == BAD_SOCKET);
    }

    bool UDP::rxAllocated()
    {
        return !(rx == BAD_SOCKET);
    }

    len_t UDP::receive()
    {
        if (!rxAllocated())
        {
            std::cerr << "RX NOT ALLOCATED YET TRYING RECEIVE!\n";
            return false;
        }
        rx_bytes = recv(rx, buf, BUF_LEN - 1, MSG_PEEK);
        if (rx_bytes == -1)
        {
            std::cerr << "recvform error\n";
            return rx_bytes;
        } else if (rx_bytes == 0) {
            std::cerr << "recvfrom closed?\n";;
        }
        buf[rx_bytes] = '\0';
        return rx_bytes;
    }

    bool UDP::transmit(std::string_view msg)
    {
        if (tx_bytes = sendto(tx, msg.data(), static_cast<msg_len_t>(msg.length()), SENDTO_FLAG,
                              target->ai_addr, static_cast<ai_addrlen_t>(target->ai_addrlen));
            tx_bytes == -1)
        {
            std::cerr << "send error\n";
            return false;
        }
        std::cout << "send successful\n";
        return true;
    }

    void UDP::clearAll()
    {
        if (rx != BAD_SOCKET)
        {
#ifdef UDPNS_WINDOWS
            closesocket(rx);
#else
            close(rx);
#endif
            rx = BAD_SOCKET;
        }
        if (tx != BAD_SOCKET)
        {
#ifdef UDPNS_WINDOWS
            closesocket(tx);
#else
            close(tx);
#endif
            tx = BAD_SOCKET;
        }
        if (buf)
        {
            delete[] buf;
            buf = nullptr;
        }
        freeResults();
#ifdef UDPNS_WINDOWS
        WSACleanup();
#endif
    }

    UDP::~UDP()
    {
        clearAll();
    }

} // namespace UDPNS
