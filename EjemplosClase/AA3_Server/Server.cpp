#include <string>
#include <iostream>  
#include <SFML/Network.hpp>

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;	
	unsigned short port;
};

sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, Client> clientList;

int main()
{
	std::map<std::string, Client>::iterator it = clientList.begin();
	socketStatus = udpSocket.bind(50000);
	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;

		socketStatus = udpSocket.receive(pack, ip, port);
		if (socketStatus != sf::UdpSocket::Status::Done)
		{
			// TODO: WHAT DO WE HANDLE HERE?
			std::cout << "Socket status is " << socketStatus;
		}
		else
		{
			std::string auxPlayerID = ip.toString() + ":" + std::to_string(port);
			if (clientList.find(auxPlayerID) == clientList.end())
			{
				// INSERT INTO MAP
				clientList.insert(std::pair<std::string, Client>(auxPlayerID, Client(ip, port)));
			}

			std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a message: " << pack << std::endl;

			// WE SEND THE PACK TO ALL CONNECTED CLIENTS
			// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
			for (std::pair<std::string, Client> element : clientList) {
				socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
			}
		}


	}

	return 0;
}