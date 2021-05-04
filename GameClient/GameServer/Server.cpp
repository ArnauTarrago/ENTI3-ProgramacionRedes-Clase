#include <string>
#include <thread>
#include <iostream>  
#include <sstream>
#include <timer.h>
#include <SFML/Network.hpp>
#include "shared.h"

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;
	unsigned short port;
	long long int clientSalt;
	long long int serverSalt;
	Timer inactivityTimer;
};

// --- Global variables ---
sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, Client> clientList;
bool executing;
// -----------------------

#pragma region Operations

void DisconnectClient(sf::IpAddress ip, unsigned short port)
{
	sf::Packet pack;

	for (std::pair<std::string, Client> element : clientList) {
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_CLIENT_HAS_DISCONNECTED << element.second.clientSalt << element.second.serverSalt << ip.toInteger() << port;
		socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
	}
}
#pragma endregion


bool IsClientSaltValid(std::string clientID, long long int auxClientSalt)
{
	return auxClientSalt == clientList.at(clientID).clientSalt;
}

bool IsServerSaltValid(std::string clientID, long long int auxServerSalt)
{
	return auxServerSalt == clientList.at(clientID).serverSalt;
}

bool AreSaltsValid(long long int auxServerSalt, std::string clientID, long long int auxClientSalt)
{
	return IsServerSaltValid(clientID, auxServerSalt) && IsClientSaltValid(clientID, auxClientSalt);
}

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
			std::cout << "Closing server." << endl;
			return;
		}
		else {
			std::cout << "Command not recognized." << endl;
		}
	}
}

void TimerCheck()
{
	while (executing)
	{
		for (std::map<std::string, Client>::iterator it = clientList.begin(); it != clientList.end();)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > 5)
			{
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
				sf::String auxClientID = it->second.ip.toString() + ":" + std::to_string(it->second.port);

				DisconnectClient(it->second.ip, it->second.port);
				it = clientList.erase(it);
			}
			else
			{
				++it;
			}
			
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
					clientList.at(auxClientID).serverSalt = Utilities::GenerateSalt();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHALLENGE << clientList.at(auxClientID).clientSalt << clientList.at(auxClientID).serverSalt;
					socketStatus = udpSocket.send(pack, ip, port);
					break;
				case CHALLENGE_R:
					pack >> auxClientSalt >> auxServerSalt;
					
					if (AreSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
						pack.clear();
						// START THE INACTIVITY TIMER
						clientList.at(auxClientID).inactivityTimer.start();
						pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << clientList.at(auxClientID).clientSalt << clientList.at(auxClientID).serverSalt;
						socketStatus = udpSocket.send(pack, ip, port);
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case CHAT_CLIENT_TO_SERVER:
					pack >> auxClientSalt >> auxServerSalt >> message;
					if (AreSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						// RESET THE INACTIVITY TIMER
						clientList.at(auxClientID).inactivityTimer.start();

						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << "/" << auxServerSalt << " and message:" << message << std::endl;

						// WE SEND THE PACK TO ALL CONNECTED CLIENTS
						// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
						for (std::pair<std::string, Client> element : clientList) {
							if (element.second.ip != ip || element.second.port != port) {
								pack.clear();
								pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHAT_SERVER_TO_CLIENT << element.second.clientSalt << element.second.serverSalt << ip.toInteger() << sf::Uint32(port) << message;
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
					if (AreSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
						
						DisconnectClient(ip, port);						
						clientList.erase(auxClientID);
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
	socketStatus = udpSocket.bind(50000);
	std::cout << "Awaiting connections from clients..." << endl;
	std::thread tCommands(&commands);
	std::thread tReceive(&receive);
	std::thread tTimerCheck(&TimerCheck);

	while (executing)
	{

	}

	udpSocket.unbind();
	tCommands.join();
	tReceive.join(); 
	tTimerCheck.join();

	sf::Packet pack;
	for (std::map<std::string, Client>::iterator it = clientList.begin(); it != clientList.end(); it++)
	{
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_SERVER << it->second.clientSalt << it->second.serverSalt;
		socketStatus = udpSocket.send(pack, it->second.ip, it->second.port);
	}

	return 0;
}