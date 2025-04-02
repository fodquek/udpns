#include <iostream>
#include <string_view>
#include <thread>
#include "udpns.h"

void receiveThread(std::string_view serv)
{
    UDPNS::UDP udpRX{};
    udpRX.createRX("", serv);
    if (!udpRX.makeBuffers())
    {
        std::cerr << "MAKEBUFFER!!\n";
        return;
    }
    std::cout << "RX UP\n";
    while (true)
    {
        if (!udpRX.receive())
        {
            std::cerr << "RECEIVE ERR\n";
            return;
        }
        std::cout << ">>> " << udpRX.buf << " <<<\n";
        if (std::string_view(udpRX.buf) == "KAPAT")
        {
            std::cout << "RX KAPAT GELDI\n";
            break;
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    if (argc != 4)
    {
        std::cerr << "USAGE: peer RX_PORT TX_HOST TX_PORT\n";
        return -1;
    }

#ifdef UDPNS_WINDOWS
    if (!UDPNS::initWSA())
    {
        std::cerr << "WSA OLMUYOKEEEE\n";
        return -1;
    }
#endif

    std::jthread rxHandle(receiveThread, argv[1]);

    UDPNS::UDP udpTX{};
    udpTX.createTX(argv[2], argv[3]);
    std::cout << "TX UP\n";
    while (true)
    {
        std::string msg;
        std::cin >> msg;
        udpTX.transmit(msg);
        if (msg == "KAPAT")
        {
            std::cout << "TX KAPAT GELDI\n";
            break;
        }
    }

#ifdef UDPNS_WINDOWS
    WSACleanup();
#endif

    return 0;
}