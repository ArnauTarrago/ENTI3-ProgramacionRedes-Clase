#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#include <SFML/Network.hpp>
#include <iostream>
#include <random>
#include <cmath>


using namespace std;
using namespace sf;

enum COMMUNICATION_HEADER_CLIENT_TO_SERVER
{
	HELLO,
	CHALLENGE_R,
	CHAT_CLIENT_TO_SERVER
};

enum COMMUNICATION_HEADER_SERVER_TO_CLIENT
{
	CHALLENGE,
	WELCOME,
	CHAT_SERVER_TO_CLIENT
};

enum CLIENT_STATUS
{
	DISCONNECTED,
	CONNECTING,
	CONNECTED
};

static class Utilities
{
public: 
	static long long int GenerateSalt()
	{
		std::random_device rd;

		std::mt19937_64 e2(rd());

		std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

		return dist(e2);
	}
};


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
