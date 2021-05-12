#include <string>
#include <thread>
#include <iostream>  
#include <sstream>
#include <timer.h>
#include <Constants.h>
#include <SFML/Network.hpp>
#include "shared.h"
#include <GameInfo.h>
#include <list>
#include <mutex>

/*
localhost
50000

*/

#pragma region Data Structures
struct PreInfo
{
	PreInfo(sf::IpAddress _ip, unsigned short _port, long long int _clientSalt, long long int _serverSalt, Timer _inactivityTimer, int _challenge) : ip(_ip), port(_port),clientSalt(_clientSalt) {};
	sf::IpAddress ip;
	unsigned short port;
	long long int clientSalt;
	long long int serverSalt;
	Timer inactivityTimer;
	int challenge;
};

struct Client
{
	sf::IpAddress ip;
	unsigned short port;
	long long int clientSalt;
	long long int serverSalt;
	Timer inactivityTimer;
	Cell position;

	Client(sf::IpAddress _ip, unsigned short _port, long long int _clientSalt, long long int _serverSalt, Timer _inactivityTimer, Cell _position) 
		: ip(_ip), port(_port), clientSalt(_clientSalt), serverSalt(_serverSalt), inactivityTimer(_inactivityTimer) {position.x = _position.x; position.y = _position.y; };

};

struct CriticalPacket
{
	sf::Packet pack;
};
#pragma endregion

#pragma region Global Variables
sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, PreInfo> connectingClientsList;
std::map<int, Client> connectedClientsList;
std::map<long int, CriticalPacket> listMsgNonAck;
Timer resendUnvalidatedPacketsTimer;
std::mutex sv_semaphore;
long int localPacketID = 0;
int localClientID = 0;
bool executing;
#pragma endregion

#pragma region Operations

void SendDisconnectMessageToConnectedClients(sf::IpAddress ip, unsigned short port)
{
	sf::Packet pack;

	for (std::pair<int, Client> element : connectedClientsList) {
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_CLIENT_HAS_DISCONNECTED << element.second.clientSalt << element.second.serverSalt << ip.toInteger() << port;
		socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
	}
}

int GenerateChallenge()
{
	/*std::random_device rd;

	std::mt19937_64 e2(rd());

	std::uniform_int_distribution<int> dist(std::llround(std::pow(2, 5)), std::llround(std::pow(2, 6)));

	return dist(e2);*/

	return 5;
}

int GenerateClientID()
{
	int auxClientID = localClientID;
	localClientID++;
	if (localClientID == numeric_limits<int>::max())
	{
		localClientID = 0;
	}
	return auxClientID;
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

bool IsConnectedClientClientSaltValid(int clientID, long long int auxClientSalt)
{
	return auxClientSalt == connectedClientsList.at(clientID).clientSalt;
}

bool IsConnectedClientServerSaltValid(int clientID, long long int auxServerSalt)
{
	return auxServerSalt == connectedClientsList.at(clientID).serverSalt;
}

bool AreConnectedClientSaltsValid(long long int auxServerSalt, int clientID, long long int auxClientSalt)
{
	return IsConnectedClientServerSaltValid(clientID, auxServerSalt) && IsConnectedClientClientSaltValid(clientID, auxClientSalt);
}

bool IsChallengeResponseValid(int challenge, int challengeResponse)
{
	return challengeResponse == challenge + 1;
}

bool IsClientIdInt(COMMUNICATION_HEADER_CLIENT_TO_SERVER header)
{
	switch (header)
	{
	case HELLO:
	case CHALLENGE_R:
		return false;
		break;
	case CHAT_CLIENT_TO_SERVER:
	case DISCONNECT_CLIENT:
	case ACKNOWLEDGE:
		return true;
		break;
	default:
		return false;
		break;
	}
}
#pragma endregion

#pragma region Threads
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
	std::list<int> auxConnectedClientsList;
	std::list<std::string> auxConnectingClientsList;
	while (executing)
	{		
		for (std::map<int, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); ++it)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_WHILE_CONNECTED_IN_SECONDS)
			{
				auxConnectedClientsList.push_back(it->first);
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
				SendDisconnectMessageToConnectedClients(it->second.ip, it->second.port);
			}
		}

		sv_semaphore.lock();
		for (std::list<int>::iterator it = auxConnectedClientsList.begin(); it != auxConnectedClientsList.end(); ++it)
		{			
			connectedClientsList.erase(*it);
		}
		sv_semaphore.unlock();

		auxConnectedClientsList.clear();

		
		for (std::map<std::string, PreInfo>::iterator it = connectingClientsList.begin(); it != connectingClientsList.end(); ++it)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > TIMER_SERVER_CHECK_FOR_CLIENT_INACTIVITY_DURING_CONNECTION_IN_SECONDS)
			{
				auxConnectingClientsList.push_back(it->first);
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
			}
		}

		sv_semaphore.lock();
		for (std::list<std::string>::iterator it = auxConnectingClientsList.begin(); it != auxConnectingClientsList.end(); ++it)
		{
			connectingClientsList.erase(*it);
		}
		sv_semaphore.unlock();

		auxConnectingClientsList.clear();

		/*if (resendUnvalidatedPacketsTimer.elapsedMilliseconds() >= TIMER_RESEND_CRITICAL_PACKETS_IN_MILISECONDS)
		{
			for (std::map<long int, CriticalPacket>::iterator it = listMsgNonAck.begin(); it != listMsgNonAck.end();)
			{
				
			}

			resendUnvalidatedPacketsTimer.start();
		}*/
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
				int auxClientID;
				std::string auxClientIpPort;

				pack >> auxClientSalt >> auxServerSalt;

				if (IsClientIdInt(auxCommHeader))
				{
					pack >> auxClientID;
					// WE CHECK IF THE CLIENT EXISTS IN THE MAP
					if (connectedClientsList.find(auxClientID) == connectedClientsList.end())
					{
						// TODO: ASK QUESTION: WHAT HAPPENS IF THE CLIENT ISN'T IN THE LIST OF CONNECTED CLIENTS?
						continue;
					}
				}
				else
				{
					auxClientIpPort = ip.toString() + ":" + std::to_string(port);
					// WE CHECK IF THE CLIENT EXISTS IN THE MAP
					if (connectingClientsList.find(auxClientIpPort) == connectingClientsList.end()) // if it doesn't exist
					{
						// if client doesn't exist, we insert it into the map
						Timer inactivityTimer;
						inactivityTimer.start();
						sv_semaphore.lock();
						connectingClientsList.insert(std::pair<std::string, PreInfo>(auxClientIpPort, PreInfo(ip, port,0,0,inactivityTimer,0)));
						sv_semaphore.unlock();
					}
				}
				switch (auxCommHeader)
				{
				case HELLO:
				{
					connectingClientsList.at(auxClientIpPort).clientSalt = auxClientSalt;

					std::cout << "Client with IP: " << ip.toString() << " and port " << port << " has sent a HELLO message with salt: " << connectingClientsList.at(auxClientIpPort).clientSalt << std::endl;
					pack.clear();
					connectingClientsList.at(auxClientIpPort).serverSalt = Utilities::GenerateSalt();
					connectingClientsList.at(auxClientIpPort).challenge = GenerateChallenge();
					pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CHALLENGE << connectingClientsList.at(auxClientIpPort).clientSalt << connectingClientsList.at(auxClientIpPort).serverSalt << connectingClientsList.at(auxClientIpPort).challenge;
					socketStatus = udpSocket.send(pack, ip, port);
				}
					break;
				case CHALLENGE_R:
					int challengeResponse;

					pack >> challengeResponse;					
					if (AreConnectingClientSaltsValid(auxServerSalt, auxClientIpPort, auxClientSalt))
					{					
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
						if (IsChallengeResponseValid(connectingClientsList.at(auxClientIpPort).challenge, challengeResponse))
						{
							std::cout << "[CHALLENGE RESPONSE VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHALLENGE_R message with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
							pack.clear();

							// TODO: MAKE STARTING POSITION RANDOM
							Cell position;
							position.x = 1;
							position.y = 1;

							// WE GENERATE A NEW CLIENT ID
							int auxNewClientID = GenerateClientID();

							// WE TELL THE CURRENTLY CONNECTED CLIENTS THAT A NEW CLIENT HAS CONNECTED
							for (std::pair<int, Client> element : connectedClientsList) {
								pack.clear();
								pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::NEW_CLIENT << element.second.clientSalt << element.second.serverSalt << connectingClientsList.at(auxClientIpPort).ip.toInteger() << connectingClientsList.at(auxClientIpPort).port << position.x << position.y;
								socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
								pack.clear();
							}

							pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << connectingClientsList.at(auxClientIpPort).clientSalt << connectingClientsList.at(auxClientIpPort).serverSalt << auxNewClientID << connectedClientsList.size();
							for (std::pair<int, Client> element : connectedClientsList) {
								pack << element.first << element.second.position.x << element.second.position.y;
							}
							socketStatus = udpSocket.send(pack, ip, port);
							pack.clear();

							// WE CREATE A NEW CLIENT AND PUT IT INTO THE CONNECTED CLIENTS MAP
							Timer inactivityTimer;
							inactivityTimer.start();							
							Client newClient = Client(connectingClientsList.at(auxClientIpPort).ip, connectingClientsList.at(auxClientIpPort).port, connectingClientsList.at(auxClientIpPort).clientSalt, connectingClientsList.at(auxClientIpPort).serverSalt, inactivityTimer, position);

							sv_semaphore.lock();
							connectedClientsList.insert(std::pair<int, Client>(auxNewClientID, newClient));
							sv_semaphore.unlock();

							// WE ERASE THE ENTRY FROM THE CONNECTING CLIENTS MAP
							sv_semaphore.lock();
							connectingClientsList.erase(auxClientIpPort);
							sv_semaphore.unlock();
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

					pack >> message;
					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						// RESET THE INACTIVITY TIMER
						connectedClient.inactivityTimer.start();

						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a CHAT message with salt " << auxClientSalt << "/" << auxServerSalt << " and message:" << message << std::endl;

						// WE SEND THE PACK TO ALL CONNECTED CLIENTS
						// FOR NOW, THIS ALSO SENDS THE PACKET TO THE ORIGINAL CLIENT SENDER
						for (std::pair<int, Client> element : connectedClientsList) {
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
					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
						
						SendDisconnectMessageToConnectedClients(ip, port);						
						connectedClientsList.erase(auxClientID);
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case ACKNOWLEDGE:
					sf::Int32 packetID;

					pack >> packetID;
					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent an ACKNOWLEDGE message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;

						listMsgNonAck.erase(packetID);
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent an ACKNOWLEDGE message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}
#pragma endregion

int main()
{
	executing = true;
	socketStatus = udpSocket.bind(50000);
	std::cout << "Awaiting connections from clients..." << endl;
	std::thread tCommands(&commands);
	std::thread tReceive(&receive);
	std::thread tTimerCheck(&TimerCheck);

	resendUnvalidatedPacketsTimer.start();

	while (executing)
	{

	}

	udpSocket.unbind();
	tCommands.join();
	tReceive.join(); 
	tTimerCheck.join();

	sf::Packet pack;
	for (std::map<int, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); it++)
	{
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_SERVER << it->second.clientSalt << it->second.serverSalt;
		socketStatus = udpSocket.send(pack, it->second.ip, it->second.port);
	}

	return 0;
}