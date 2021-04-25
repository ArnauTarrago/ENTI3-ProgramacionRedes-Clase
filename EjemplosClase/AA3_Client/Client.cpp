#include <string>
#include <SFML/Network.hpp>


sf::UdpSocket udpSocket;
sf::IpAddress serverIP = "localhost";
unsigned short serverPort = 50000;

int main()
{
	sf::Packet pack;

	// WE SEND A MESSAGE TO THE SERVER SO IT KNOWS WHO WE ARE
	udpSocket.send(pack, serverIP, serverPort);
	while (true)
	{
		udpSocket.receive(pack, serverIP, serverPort);

		// TODO: HANDLE THE TYPE OF MESSAGE SENT BY THE SERVER
	}


	return 0;
}