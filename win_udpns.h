#ifndef H_UDPNS
#define H_UDPNS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <string_view>

namespace UDPNS
{

    extern const int GAI_OK;
    extern const int BIND_ERROR;
    extern const int RECV_FORM_FLAG;
    extern const int SENDTO_FLAG;
    extern const int RECVFORM_FLAG;
    extern const std::string_view MY_PORT;
    extern const size_t BUF_LEN;
    extern const int ADDR_LEN;

    void *get_in_addr(struct sockaddr *sa);
    bool initWSA();

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
        SOCKET tx{INVALID_SOCKET};
        SOCKET rx{INVALID_SOCKET};
        struct addrinfo *results{};
        struct addrinfo *target{};
        char *buf{nullptr};
        size_t rx_bytes{};
        size_t tx_bytes{};
    };

} // namespace UDPNS

#endif
