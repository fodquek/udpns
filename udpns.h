#ifndef H_UDPNS
#define H_UDPNS

#include <arpa/inet.h>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string_view>

#include <fcntl.h>
// #include <errno.h>

namespace UDPNS {

extern const int GAI_OK;
extern const int SOCKET_ERROR;
extern const int BIND_ERROR;
extern const int RECV_FORM_FLAG;
extern const int SENDTO_FLAG;
extern const int RECVFORM_FLAG;
extern const std::string_view MY_PORT;
extern const size_t BUF_LEN;
extern const int ADDR_LEN;


void* get_in_addr(struct sockaddr* sa); 

class UDP{
public:
    explicit UDP() = default;
    
    // no copy-ctor
    UDP(const UDP& arg) = delete; 
    // no copy-assignment
    UDP& operator=(const UDP& rhs) = delete; 
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

//private: ŞİMDİLİK DEĞİLLER SONRADAN EKLICEZKE, ELEGANS OLCAK
    int tx {SOCKET_ERROR};
    int rx {SOCKET_ERROR};
    struct addrinfo* results {};
    struct addrinfo* target {};
    char* buf;
    ssize_t rx_bytes {};
    ssize_t tx_bytes {};
};




/*
tx socket için hedef ip belirtilmeli
TODO: ilerde otomatik arama eklenebilir?
*/
int createTX(std::string_view host_ip);
} // namespace UDPNS

#endif