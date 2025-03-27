#ifndef H_UDPNS
#define H_UDPNS

#ifndef UDPNS_WINDOWS
#define UDPNS_WINDOWS
#endif

#ifdef UDPNS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <string_view>
#include <iostream>

namespace UDPNS
{

#ifdef UDPNS_WINDOWS
    using sockfd_t = SOCKET;
#else
    using sockfd_t = int;
#endif

const int GAI_OK{0};
#ifdef UDPNS_WINDOWS
    const sockfd_t BAD_SOCKET{INVALID_SOCKET};
    const int BAD_BIND{SOCKET_ERROR};
#else
    const sockfd_t BAD_SOCKET{-1};
    const int BAD_BIND{-1};
#endif
    const int SENDTO_FLAG{0};
    const int RECVFORM_FLAG{0};
    const std::string_view MY_PORT{"4951"};
    const size_t BUF_LEN{100};
    const int ADDR_LEN{18};

    void *get_in_addr(struct sockaddr *sa);
#ifdef UDPNS_WINDOWS
    bool initWSA();
#endif

    class UDP
    {
    public:
        explicit UDP() = default;

        // no copy-ctor
        UDP(const UDP &arg) = delete;
        // no copy-assignment
        UDP &operator=(const UDP &rhs) = delete;
        // get sockaddr, IPv4 or IPv6:
        ~UDP();

        [[nodiscard("MALLOC VARKE")]] bool makeBuffers();

        bool gai(std::string_view host, std::string_view port);

        bool createSocket(std::string_view host, std::string_view port, bool RX);
        bool createRX(std::string_view host, std::string_view port);
        bool createTX(std::string_view host, std::string_view port);
        void freeResults();

        bool txAllocated();
        bool rxAllocated();

        bool receive();
        bool transmit(std::string_view msg);

        void clearAll();

        // private: ŞİMDİLİK DEĞİLLER SONRADAN EKLICEZKE, ELEGANS OLCAK
        sockfd_t tx{BAD_SOCKET};
        sockfd_t rx{BAD_SOCKET};
        struct addrinfo *results{};
        struct addrinfo *target{};
        char *buf{nullptr};
        size_t rx_bytes{};
        size_t tx_bytes{};
    };
} // namespace UDPNS

#endif