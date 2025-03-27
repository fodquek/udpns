#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// https://learn.microsoft.com/en-us/windows/win32/winsock/creating-a-basic-winsock-application
#include "win_udpns.h"
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#define DEFAULT_BUFLEN 512
char recvbuf[DEFAULT_BUFLEN];
#define DEFAULT_PORT "27015"
int recvbuflen{ DEFAULT_BUFLEN };
const std::string_view sendbuf{ "this is a test" };

void receiveThread(std::string_view serv)
{
    
    UDPNS::UDP udpRX{};
    udpRX.createRX("", serv);
    if (!udpRX.makeBuffers()) {
        std::cerr << "MAKEBUFFER!!\n";
        return;
    }
    std::cout << "RX UP\n";
    while (true) {
        if (!udpRX.receive()) {
            std::cerr << "RECEIVE ERR\n";
            return;
        }
        std::cout << ">>> " << udpRX.buf << " <<<\n";
        if (std::string_view(udpRX.buf) == "KAPAT") {
            std::cout << "RX KAPAT GELDI\n";
            break;
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    if (!UDPNS::initWSA()) {
        std::cerr << "WSA OLMUYOKEEEE\n";
        return -1;
    }

    if (argc != 4) {
        std::cerr << "USAGE: peer RX_PORT TX_HOST TX_PORT\n";
        return -2;
    }

    std::jthread rxHandle(receiveThread, argv[1]);

    UDPNS::UDP udpTX{};
    udpTX.createTX(argv[2], argv[3]);
    std::cout << "TX UP\n";
    while (true) {
        std::string msg;
        std::cin >> msg;
        udpTX.transmit(msg);
        if (msg == "KAPAT") {
            std::cout << "TX KAPAT GELDI\n";
            break;
        }
    }

    WSACleanup();
    return 0;
}