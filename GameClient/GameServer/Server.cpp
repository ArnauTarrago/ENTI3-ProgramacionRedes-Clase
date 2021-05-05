#include <string>
#include <thread>
#include <iostream>  
#include <sstream>
#include <timer.h>
#include <Constants.h>
#include <SFML/Network.hpp>
#include "shared.h"

/* Dubtes
-Quina informació posem pels que s'estan conectant?
-Si client diu algo, comprovem que esta tant en el map de conectats com els de conectant?
-"Reservar espai"?
*/

struct PreInfo
{
	PreInfo(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	sf::IpAddress ip;
	unsigned short port;
	long long int clientSalt;
	long long int serverSalt;
	Timer inactivityTimer;
	int challenge;
};

struct Client
{
	Client(sf::IpAddress _ip, unsigned short _port) : ip(_ip), port(_port) {};
	Client(sf::IpAddress _ip, unsigned short _port, long long int _clientSalt, long long int _serverSalt, Timer _inactivityTimer) 
		: ip(_ip), port(_port), clientSalt(_clientSalt), serverSalt(_serverSalt), inactivityTimer(_inactivityTimer){};
	sf::IpAddress ip;
	unsigned short port;
	long long int clientSalt;
	long long int serverSalt;
	Timer inactivityTimer;
};

// --- Global variables ---
sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, PreInfo> connectingClientsList;
std::map<std::string, Client> connectedClientsList;
bool executing;
// -----------------------

#pragma region Operations

void DisconnectClient(sf::IpAddress ip, unsigned short port)
{
	sf::Packet pack;

	for (std::pair<std::string, Client> element : connectedClientsList) {
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_CLIENT_HAS_DISCONNECTED << element.second.clientSalt << element.second.serverSalt << ip.toInteger() << port;
		socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
	}
}
#pragma endregion

#pragma region Validations
bool IsConnectingClientClientSaltValid(std::string clientID, long long int auxClientSalt)
{
	return auxClientSalt == connectingClientsList.at(clientID).clientSalt;
}

bool IsConnectingClientServerSaltValid(std::string clientID, long long int auxServerSalt)
{
	return auxServerSalt == connectingClientsList.at(clientID).serverSalt;
}

bool AreConnectingClientSaltsValid(long long int auxServerSalt, std::string clientID, long long int auxClientSalt)
{
	return IsConnectingClientServerSaltValid(clientID, auxServerSalt) && IsConnectingClientClientSaltValid(clientID, auxClientSalt);
}

bool IsConnectedClientClientSaltValid(std::string clientID, long long int auxClientSalt)
{
	return auxClientSalt == connectedClientsList.at(clientID).clientSalt;
}

bool IsConnectedClientServerSaltValid(std::string clientID, long long int auxServerSalt)
{
	return auxServerSalt == connectedClientsList.at(clientID).serverSalt;
}

bool AreConnectedClientSaltsValid(long long int auxServerSalt, std::string clientID, long long int auxClientSalt)
{
	return IsConnectedClientServerSaltValid(clientID, auxServerSalt) && IsConnectedClientClientSaltValid(clientID, auxClientSalt);
}

bool IsChallengeResponseValid(int challenge, int challengeResponse)
{
	return challengeResponse == challenge + 1;
}

#pragma endregion

int GenerateChallenge()
{
	/*std::random_device rd;

	std::mt19937_64 e2(rd());

	std::uniform_int_distribution<int> dist(std::llround(std::pow(2, 5)), std::llround(std::pow(2, 6)));

	return dist(e2);*/

	return 5;
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
		for (std::map<std::string, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end();)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > TIMER_SERVER_CHECK_FOR_CLIENT_INACTIVITY_WHILE_CONNECTED_IN_SECONDS)
			{
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
				sf::String auxClientID = it->second.ip.toString() + ":" + std::to_string(it->second.port);

				DisconnectClient(it->second.ip, it->second.port);
				it = connectedClientsList.erase(it);
			}
			else
			{
				++it;
			}			
		}

		for (std::map<std::string, PreInfo>::iterator it = connectingClientsList.begin(); it != connectingClientsList.end();)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > TIMER_SERVER_CHECK_FOR_CLIENT_INACTIVITY_DURING_CONNECTION_IN_SECONDS)
			{
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
				sf::String auxClientID = it->second.ip.toString() + ":" + std::to_string(it->second.port);
				it = connectingClientsList.erase(it);
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
				if (connectedClientsList.find(auxClientID) == connectedClientsList.end()) // IF IT DOESN'T EXIST
				{
					if (connectingClientsList.find(auxClientID) == connectingClientsList.end()) // IF IT DOESN'T EXIST
					{
						// IF CLIENT DOESN'T EXIST, WE INSERT IT INTO THE MAP
						connectingClientsList.insert(std::pair<std::string, PreInfo>(auxClientID, PreInfo(ip, port)));
					}					
				}

				switch (auxCommHeader)
				{
				case HELLO:
				{
					pack >> connectingClientsList.at(auxClientID).clientSalt;
					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a HELLO message with salt: " << connectingClientsList.at(auxClientID).clientSalt << std::endl;
					pack.clear();
					connectingClientsList.at(auxClientID).serverSalt = Utilities::GenerateSalt();
					connectingClientsList.at(auxClientID).challenge = GenerateChallenge();
					connectingClientsList.at(auxClientID).inactivityTimer.start();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHALLENGE << connectingClientsList.at(auxClientID).clientSalt << connectingClientsList.at(auxClientID).serverSalt << connectingClientsList.at(auxClientID).challenge;
					socketStatus = udpSocket.send(pack, ip, port);
				}
					break;
				case CHALLENGE_R:
					int challengeResponse;

					pack >> auxClientSalt >> auxServerSalt >> challengeResponse;					
					if (AreConnectingClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						

						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
						if (IsChallengeResponseValid(connectingClientsList.at(auxClientID).challenge, challengeResponse))
						{
							std::cout << "[CHALLENGE RESPONSE VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
							pack.clear();

							// WE CREATE A NEW CLIENT AND PUT IT INTO THE CONNECTED CLIENTS MAP
							Timer inactivityTimer;
							inactivityTimer.start();
							Client newClient = Client(connectingClientsList.at(auxClientID).ip, connectingClientsList.at(auxClientID).port, connectingClientsList.at(auxClientID).clientSalt, connectingClientsList.at(auxClientID).serverSalt, inactivityTimer);
							connectedClientsList.insert(std::pair<std::string, Client>(auxClientID, newClient));
							
							pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << connectingClientsList.at(auxClientID).clientSalt << connectingClientsList.at(auxClientID).serverSalt;
							
							// WE ERASE THE ENTRY FROM THE CONNECTING CLIENTS MAP
							connectingClientsList.erase(auxClientID);

							socketStatus = udpSocket.send(pack, ip, port);
						}
						else
						{
							std::cout << "[CHALLENGE RESPONSE INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
						}						
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case CHAT_CLIENT_TO_SERVER:
				{


					Client connectedClient = connectedClientsList.at(auxClientID);

					pack >> auxClientSalt >> auxServerSalt >> message;
					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{


						// RESET THE INACTIVITY TIMER
						connectedClient.inactivityTimer.start();

						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << "/" << auxServerSalt << " and message:" << message << std::endl;

						// WE SEND THE PACK TO ALL CONNECTED CLIENTS
						// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
						for (std::pair<std::string, Client> element : connectedClientsList) {
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
				}
					break;
				case DISCONNECT_CLIENT:
					pack >> auxClientSalt >> auxServerSalt;
					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
						
						DisconnectClient(ip, port);						
						connectedClientsList.erase(auxClientID);
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
	for (std::map<std::string, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); it++)
	{
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_SERVER << it->second.clientSalt << it->second.serverSalt;
		socketStatus = udpSocket.send(pack, it->second.ip, it->second.port);
	}

	return 0;
}