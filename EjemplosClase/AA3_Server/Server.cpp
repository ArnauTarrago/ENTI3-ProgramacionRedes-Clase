#include <string>
#include <iostream>  
#include <SFML/Network.hpp>
#include "../shared.h"

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;	
	unsigned short port;
	long long int clientSalt;
};

sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, Client> clientList;
long long int serverSalt;

int main()
{
	sf::Packet pack;
	sf::IpAddress ip;
	unsigned short port;
	int msg;
	COMMUNICATION_HEADER_CLIENT_TO_SERVER auxCommHeader;

	serverSalt = Utilities::GenerateSalt();
	std::cout << "Generated salt: " << serverSalt << endl;
	std::map<std::string, Client>::iterator it = clientList.begin();
	socketStatus = udpSocket.bind(50000);
	std::cout << "Awaiting connections from clients..." << endl;

	while (true)
	{
		socketStatus = udpSocket.receive(pack, ip, port);
		if (socketStatus != sf::UdpSocket::Status::Done)
		{
			// TODO: WHAT DO WE HANDLE HERE?
			std::cout << "Socket status is " << socketStatus << endl;
		}
		else
		{
			if (pack >> msg)
			{
				std::string chatMessage;
				auxCommHeader = (COMMUNICATION_HEADER_CLIENT_TO_SERVER)msg;
				long long int auxClientSalt, auxServerSalt;

				// WE CHECK IF THE CLIENT EXISTS IN THE MAP
				std::string auxPlayerID = ip.toString() + ":" + std::to_string(port);
				if (clientList.find(auxPlayerID) == clientList.end())
				{
					// IF CLIENT DOESN'T EXIST, WE INSERT IT INTO THE MAP
					clientList.insert(std::pair<std::string, Client>(auxPlayerID, Client(ip, port)));
				}				
				
				switch (auxCommHeader)
				{
				case HELLO:
					pack >> clientList.at(auxPlayerID).clientSalt;
					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a HELLO message with salt: " << clientList.at(auxPlayerID).clientSalt << std::endl;					
					pack.clear();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHALLENGE << clientList.at(auxPlayerID).clientSalt << serverSalt;
					socketStatus = udpSocket.send(pack, ip, port);
					break;
				case CHALLENGE_R:
					pack >> auxClientSalt >> auxServerSalt;
					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					pack.clear();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << clientList.at(auxPlayerID).clientSalt << serverSalt;
					socketStatus = udpSocket.send(pack, ip, port);
					break;
				case CHAT_CLIENT_TO_SERVER:
					
					pack >> auxClientSalt >> chatMessage;
					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << " and message:" << chatMessage << std::endl;

					pack.clear();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHAT_SERVER_TO_CLIENT << chatMessage;

					// WE SEND THE PACK TO ALL CONNECTED CLIENTS
					// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
					for (std::pair<std::string, Client> element : clientList) {
						socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
					}
					break;
				default:
					break;
				}

				
			}

			

			

			
		}


	}

	return 0;
}