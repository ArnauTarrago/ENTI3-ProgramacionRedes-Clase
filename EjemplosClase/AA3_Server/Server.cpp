#include <string>
#include <thread>
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
bool executing;

void commands() {
	while (executing) {
		std::string input;
		std::cin >> input;
		std::locale loc;
		for (std::string::size_type i = 0; i < input.length(); ++i) {
			input[i] = std::toupper(input[i], loc);
		}
		if (input == "EXIT") {
			executing = false;
			std::cout << "Closing server.";
			return;
		}
		else {
			std::cout << "Command not recognized.";
		}
	}
}

void receive() {

	sf::Packet pack;
	sf::IpAddress ip;
	unsigned short port;
	int msg;

	COMMUNICATION_HEADER_CLIENT_TO_SERVER auxCommHeader;

	while (executing)
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
				std::string message;
				auxCommHeader = (COMMUNICATION_HEADER_CLIENT_TO_SERVER)msg;
				long long int auxClientSalt, auxServerSalt;

				// WE CHECK IF THE CLIENT EXISTS IN THE MAP
				std::string auxClientID = ip.toString() + ":" + std::to_string(port);
				if (clientList.find(auxClientID) == clientList.end())
				{
					// IF CLIENT DOESN'T EXIST, WE INSERT IT INTO THE MAP
					clientList.insert(std::pair<std::string, Client>(auxClientID, Client(ip, port)));
				}

				switch (auxCommHeader)
				{
				case HELLO:
					pack >> clientList.at(auxClientID).clientSalt;
					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a HELLO message with salt: " << clientList.at(auxClientID).clientSalt << std::endl;
					pack.clear();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHALLENGE << clientList.at(auxClientID).clientSalt << serverSalt;
					socketStatus = udpSocket.send(pack, ip, port);
					break;
				case CHALLENGE_R:
					pack >> auxClientSalt >> auxServerSalt;
					if (auxClientSalt == clientList.at(auxClientID).clientSalt && auxServerSalt == serverSalt)
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
						pack.clear();
						pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << clientList.at(auxClientID).clientSalt << serverSalt;
						socketStatus = udpSocket.send(pack, ip, port);
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case CHAT_CLIENT_TO_SERVER:
					pack >> auxClientSalt >> auxServerSalt >> message;
					if (auxClientSalt == clientList.at(auxClientID).clientSalt && auxServerSalt == serverSalt)
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << "/" << auxServerSalt << " and message:" << message << std::endl;

						// WE SEND THE PACK TO ALL CONNECTED CLIENTS
						// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
						for (std::pair<std::string, Client> element : clientList) {
							if (element.second.ip != ip || element.second.port != port) {
								pack.clear();
								pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHAT_SERVER_TO_CLIENT << element.second.clientSalt << serverSalt << ip.toInteger() << sf::Uint32(port) << message;
								socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
							}
						}
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << "/" << auxServerSalt << " and message:" << message << std::endl;
					}

					break;
				case DISCONNECT_CLIENT:
					pack >> auxClientSalt >> auxServerSalt;
					if (auxClientSalt == clientList.at(auxClientID).clientSalt && auxServerSalt == serverSalt)
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;

						clientList.erase(auxClientID);

						for (std::pair<std::string, Client> element : clientList) {
							pack.clear();
							pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_CLIENT_HAS_DISCONNECTED << element.second.clientSalt << serverSalt << ip.toInteger();
							socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
						}
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				default:
					break;
				}


			}






		}


	}
}

int main()
{
	executing = true;
	serverSalt = Utilities::GenerateSalt();
	std::cout << "Generated salt: " << serverSalt << endl;
	socketStatus = udpSocket.bind(50000);
	std::cout << "Awaiting connections from clients..." << endl;
	std::thread tCommands(&commands);
	std::thread tReceive(&receive);

	while (executing)
	{

	}

	udpSocket.unbind();
	tCommands.join();
	tReceive.join();

	sf::Packet pack;
	for (std::map<std::string, Client>::iterator it = clientList.begin(); it != clientList.end(); it++)
	{
		std::cout << "[SALT VALID] Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " sending a DISCONNECT message" << std::endl;
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_SERVER << it->second.clientSalt << serverSalt;
		socketStatus = udpSocket.send(pack, it->second.ip, it->second.port);
	}

	return 0;
}