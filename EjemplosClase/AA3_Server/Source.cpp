#include <thread>
#include <sstream>
#include <iostream>    
#include <mutex>
#include <chrono>
#include <string>
#include <SFML/Network.hpp>

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;	
	unsigned short port;
};

sf::UdpSocket serverUDPSocket;
std::map<std::string, Client> clientList;

int main()
{
	serverUDPSocket.bind(50000);
	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;

		serverUDPSocket.receive(pack, ip, port);
		std::string auxPlayerID = ip.toString() + ":" + std::to_string(port);
		if (clientList.find(auxPlayerID) == clientList.end() )
		{
			// INSERT INTO MAP
			clientList.insert(std::pair<std::string, Client>(auxPlayerID, Client(ip,port)));
		}

		
	}

	return 0;
}