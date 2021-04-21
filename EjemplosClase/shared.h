#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#include <SFML/Network.hpp>
#include <iostream>

using namespace std;
using namespace sf;


class MessageManager
{
public:
    Packet lastPacket;
    Socket::Status lastStatus;

#pragma region FUNCIONES GENERALES
    Packet receive_message() {
        Packet pack;

        return pack;
    }
    bool send_message(Packet& packet_) {

        return false;
    }
#pragma endregion



};

#endif // SHARED_INCLUDED
