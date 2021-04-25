#include <string>
#include <SFML/Network.hpp>

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;	
	unsigned short port;
};

sf::UdpSocket udpSocket;
std::map<std::string, Client> clientList;

int main()
{
	std::map<std::string, Client>::iterator it = clientList.begin();
	udpSocket.bind(50000);
	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;

		udpSocket.receive(pack, ip, port);
		std::string auxPlayerID = ip.toString() + ":" + std::to_string(port);
		if (clientList.find(auxPlayerID) == clientList.end() )
		{
			// INSERT INTO MAP
			clientList.insert(std::pair<std::string, Client>(auxPlayerID, Client(ip,port)));			
		}

		// WE SEND THE PACK TO ALL CONNECTED CLIENTS
		// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
		for (std::pair<std::string, Client> element : clientList) {
			udpSocket.send(pack, element.second.ip, element.second.port);
		}

	}

	return 0;
}