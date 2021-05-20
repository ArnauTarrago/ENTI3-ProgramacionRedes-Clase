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
#include <iostream>
#include <fstream>

/*
localhost
50000

*/

#pragma region Data Structures
struct PreInfo
{
	PreInfo(sf::IpAddress _ip, unsigned short _port, unsigned long long int _clientSalt, unsigned long long int _serverSalt, Timer _inactivityTimer, int _challenge) :
		ip(_ip), port(_port),clientSalt(_clientSalt), serverSalt(_serverSalt), inactivityTimer(_inactivityTimer), challenge(_challenge) {};
	sf::IpAddress ip;
	unsigned short port;
	unsigned long long int clientSalt;
	unsigned long long int serverSalt;
	Timer inactivityTimer;
	int challenge;
};

struct Client
{
	sf::IpAddress ip;
	unsigned short port;
	unsigned long long int clientSalt;
	unsigned long long int serverSalt;
	unsigned int RTTInMilliseconds = 0;
	Timer inactivityTimer;
	Cell position;

	Client(sf::IpAddress _ip, unsigned short _port, unsigned long long int _clientSalt, unsigned long long int _serverSalt, Timer _inactivityTimer, Cell _position)
		: ip(_ip), port(_port), clientSalt(_clientSalt), serverSalt(_serverSalt), inactivityTimer(_inactivityTimer) {position.x = _position.x; position.y = _position.y; };

};

struct CriticalPacket
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Packet pack;
	std::chrono::time_point<std::chrono::steady_clock> timeSent;
	Timer resendTimer;
};

struct UnvalidatedMovePacket
{
	unsigned int moveID;
	float positionX;
	float positionY;
};
#pragma endregion

#pragma region Global Variables
sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::map<std::string, PreInfo> connectingClientsList;
std::map<unsigned int, Client> connectedClientsList;
std::map<unsigned long int, CriticalPacket> listMsgNonAck;
std::map<unsigned int,UnvalidatedMovePacket> listUnvalidatedMovePackets;
Timer timerSaveRTTToFile;
Timer timerValidatePlayerMoves;
std::mutex sv_semaphore;
unsigned long int localPacketID = 0;
unsigned int localClientID = 0;
bool executing;
#pragma endregion

#pragma region Operations

int GenerateChallenge()
{
	/*std::random_device rd;

	std::mt19937_64 e2(rd());

	std::uniform_int_distribution<int> dist(std::llround(std::pow(2, 5)), std::llround(std::pow(2, 6)));

	return dist(e2);*/

	return 5;
}

unsigned int GenerateClientID()
{
	unsigned int auxClientID = localClientID;
	localClientID++;
	if (localClientID == numeric_limits<unsigned int>::max())
	{
		localClientID = 0;
	}
	return auxClientID;
}

void AddCriticalPacketToNonAcknowledgeList(sf::Packet _pack, sf::IpAddress _ip, unsigned short _port)
{
	CriticalPacket newCriticalPacket;
	Timer timer;

	newCriticalPacket.pack = _pack;
	newCriticalPacket.ip = _ip;
	newCriticalPacket.port = _port;	
	newCriticalPacket.resendTimer = timer;
	newCriticalPacket.timeSent = std::chrono::steady_clock::now();
	newCriticalPacket.resendTimer.start();

	listMsgNonAck.insert(std::pair<unsigned long, CriticalPacket>(localPacketID, newCriticalPacket));
	localPacketID++;

	if (localPacketID == numeric_limits<unsigned long>::max())
	{
		localPacketID = 0;
	}
}

void SendDisconnectMessageToConnectedClients(sf::IpAddress ip, unsigned short port)
{
	sf::Packet pack;

	for (std::pair<unsigned int, Client> element : connectedClientsList) {
		// WE MAKE SURE TO NOT SEND THE DISCONNECT MESSAGE TO THE CLIENT THAT HAS JUST DISCONNECTED
		// OTHERWISE, WE'LL BE WAITING FOR THE ACKNOWLEDGE MESSAGE FOREVER
		if (element.second.ip != ip || element.second.port != port) 
		{
			pack.clear();
			pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_CLIENT_HAS_DISCONNECTED << element.second.clientSalt << element.second.serverSalt << sf::Uint32(localPacketID) << ip.toInteger() << port;
			AddCriticalPacketToNonAcknowledgeList(pack, ip, port);
			socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
		}

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

bool IsConnectedClientClientSaltValid(unsigned int clientID, long long int auxClientSalt)
{
	return auxClientSalt == connectedClientsList.at(clientID).clientSalt;
}

bool IsConnectedClientServerSaltValid(unsigned int clientID, long long int auxServerSalt)
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
	case MOVE:
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
	std::list<unsigned int> auxConnectedClientsList;
	std::list<unsigned int> auxUnvalidatedMovePacketsList;
	std::list<std::string> auxConnectingClientsList;
	while (executing)
	{		
		for (std::map<unsigned int, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); ++it)
		{
			if (it->second.inactivityTimer.elapsedSeconds() > TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_WHILE_CONNECTED_IN_SECONDS)
			{
				auxConnectedClientsList.push_back(it->first);
				std::cout << "Client with IP: " << it->second.ip.toString() << " and port " << it->second.port << " has been disconnected for inactivity. " << std::endl;
				SendDisconnectMessageToConnectedClients(it->second.ip, it->second.port);
			}
		}

		sv_semaphore.lock();
		for (std::list<unsigned int>::iterator it = auxConnectedClientsList.begin(); it != auxConnectedClientsList.end(); ++it)
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

		for (std::map<unsigned long int, CriticalPacket>::iterator it = listMsgNonAck.begin(); it != listMsgNonAck.end(); ++it)
		{
			if (it->second.resendTimer.elapsedMilliseconds() >= TIMER_RESEND_CRITICAL_PACKETS_DURING_LOW_TRAFFIC_IN_MILLISECONDS)
			{
				CriticalPacket auxPacket = it->second;
				auxPacket.resendTimer.start();
				auxPacket.timeSent = std::chrono::steady_clock::now();
				socketStatus = udpSocket.send(auxPacket.pack, auxPacket.ip, auxPacket.port);
				std::cout << "Resent critical packet with ID '" << it->first << "'." << std::endl;
			}
		}

		if (timerSaveRTTToFile.elapsedSeconds() >= TIMER_SERVER_SAVE_RTT_TO_FILE_IN_SECONDS)
		{
			if (connectedClientsList.size() > 0)
			{
				timerSaveRTTToFile.start();
				ofstream myfile("RTTlogs.txt", std::ios::app);
				if (myfile.is_open())
				{
					for (std::map<unsigned int, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); ++it)
					{
						myfile << "[Client " << it->first << "]: RTT of " << it->second.RTTInMilliseconds << " ms.\n";
					}
					myfile << "--------------------------------------\n";
					myfile.close();
				}
				else std::cout << "[ERROR]: UNABLE TO OPEN RTTlogs.txt FILE" << std::endl;
			}			
		}

		if (timerValidatePlayerMoves.elapsedMilliseconds() >= TIMER_SERVER_VALIDATE_PLAYER_MOVES_IN_MILLISECONDS)
		{
			sf::Packet pack;

			for (std::map<unsigned int, UnvalidatedMovePacket>::iterator it = listUnvalidatedMovePackets.begin(); it != listUnvalidatedMovePackets.end(); ++it)
			{
				if (it->second.positionX < 0) it->second.positionX = 0;
				if (it->second.positionY < 0) it->second.positionY = 0;

				pack.clear();
				pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::OKMOVE << connectedClientsList.at(it->first).clientSalt << connectedClientsList.at(it->first).serverSalt << it->second.moveID << it->second.positionX << it->second.positionY;
				socketStatus = udpSocket.send(pack, connectedClientsList.at(it->first).ip, connectedClientsList.at(it->first).port);
				pack.clear();

				for (std::pair<unsigned int, Client> element : connectedClientsList) {
					// WE MAKE SURE TO NOT SEND THE DISCONNECT MESSAGE TO THE CLIENT THAT HAS JUST DISCONNECTED
					// OTHERWISE, WE'LL BE WAITING FOR THE ACKNOWLEDGE MESSAGE FOREVER
					/*if (element.second.ip != connectedClientsList.at(it->first).ip || element.second.port != connectedClientsList.at(it->first).port)
					{*/
						pack.clear();
						pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::CLIENT_HAS_MOVED << element.second.clientSalt << element.second.serverSalt << it->first << it->second.positionX << it->second.positionY;
						//AddCriticalPacketToNonAcknowledgeList(pack, ip, port);
						socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
					//}

				}

				auxUnvalidatedMovePacketsList.push_back(it->first);
			}

			timerValidatePlayerMoves.start();
		}

		sv_semaphore.lock();
		for (std::list<unsigned int>::iterator it = auxUnvalidatedMovePacketsList.begin(); it != auxUnvalidatedMovePacketsList.end(); ++it)
		{
			listUnvalidatedMovePackets.erase(*it);
		}
		sv_semaphore.unlock();

		auxUnvalidatedMovePacketsList.clear();
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
		}
		else
		{
			if (pack >> msg)
			{
				std::string message;
				auxCommHeader = (COMMUNICATION_HEADER_CLIENT_TO_SERVER)msg;
				long long int auxClientSalt, auxServerSalt;
				unsigned int auxClientID;
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
					pack.clear();
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
							unsigned int auxNewClientID = GenerateClientID();

							// WE TELL THE CURRENTLY CONNECTED CLIENTS THAT A NEW CLIENT HAS CONNECTED
							for (std::pair<unsigned int, Client> element : connectedClientsList) {
								pack.clear();
								pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::NEW_CLIENT << element.second.clientSalt << element.second.serverSalt << sf::Uint32(localPacketID) << connectingClientsList.at(auxClientIpPort).ip.toInteger() << connectingClientsList.at(auxClientIpPort).port << position.x << position.y;
								socketStatus = udpSocket.send(pack, element.second.ip, element.second.port);
								AddCriticalPacketToNonAcknowledgeList(pack, element.second.ip, element.second.port);
								pack.clear();
							}

							pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::WELCOME << connectingClientsList.at(auxClientIpPort).clientSalt << connectingClientsList.at(auxClientIpPort).serverSalt << auxNewClientID << connectedClientsList.size();
							for (std::pair<unsigned int, Client> element : connectedClientsList) {
								pack << element.first << element.second.position.x << element.second.position.y;
							}
							socketStatus = udpSocket.send(pack, ip, port);
							pack.clear();

							// WE CREATE A NEW CLIENT AND PUT IT INTO THE CONNECTED CLIENTS MAP
							Timer inactivityTimer;
							inactivityTimer.start();							
							Client newClient = Client(connectingClientsList.at(auxClientIpPort).ip, connectingClientsList.at(auxClientIpPort).port, connectingClientsList.at(auxClientIpPort).clientSalt, connectingClientsList.at(auxClientIpPort).serverSalt, inactivityTimer, position);

							sv_semaphore.lock();
							connectedClientsList.insert(std::pair<unsigned int, Client>(auxNewClientID, newClient));
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
						for (std::pair<unsigned int, Client> element : connectedClientsList) {
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

						sv_semaphore.lock();
						connectedClientsList.erase(auxClientID);
						sv_semaphore.unlock();
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a DISCONNECT message with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case ACKNOWLEDGE:
					sf::Uint32 tempPacketID;
					unsigned long packetID;

					pack >> tempPacketID;
					packetID = tempPacketID;

					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent an ACKNOWLEDGE message of packet with ID '" << packetID << "' with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
						connectedClientsList.at(auxClientID).RTTInMilliseconds =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - listMsgNonAck.at(packetID).timeSent).count();
						sv_semaphore.lock();
						listMsgNonAck.erase(packetID);
						sv_semaphore.unlock();
					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent an ACKNOWLEDGE message of packet with ID '" << packetID <<"' with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
					}
					break;
				case MOVE:
					sf::Uint32 tempMoveID;
					unsigned long moveID;

					float positionX, positionY;

					pack >> tempMoveID >> positionX >> positionY;
					moveID = tempMoveID;

					if (AreConnectedClientSaltsValid(auxServerSalt, auxClientID, auxClientSalt))
					{
						std::cout << "[SALT VALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a MOVE message to position [" << positionX << "," << positionY << "] of packet with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
						
						// IF WE DON'T FIND A CLIENT WE CREATE THE MOVE PACKET, OTHERWISE WE ADD THE ACCUMULATED MOVE TO THE EXISTING MOVE PACKET
						if (listUnvalidatedMovePackets.find(auxClientID) == listUnvalidatedMovePackets.end()) // NOT FOUND
						{
							UnvalidatedMovePacket auxMovePacket;
							auxMovePacket.positionX = positionX;
							auxMovePacket.positionY = positionY;
							auxMovePacket.moveID = tempMoveID;

							listUnvalidatedMovePackets.insert(std::pair<unsigned int, UnvalidatedMovePacket>(auxClientID, auxMovePacket));
						}
						else // FOUND
						{
							listUnvalidatedMovePackets.at(auxClientID).positionX = positionX;
							listUnvalidatedMovePackets.at(auxClientID).positionY = positionY;
						}


						// TODO: Warn the other clients that a player has moved

					}
					else
					{
						std::cout << "[SALT INVALID] Client with IP: " << ip.toString() << " and port " << port << " has sent a MOVE message of packet with ID '" << packetID << "' with salt " << auxClientSalt << "/" << auxServerSalt << std::endl;
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

	timerSaveRTTToFile.start();
	timerValidatePlayerMoves.start();

	while (executing)
	{

	}

	udpSocket.unbind();
	tCommands.join();
	tReceive.join(); 
	tTimerCheck.join();

	sf::Packet pack;
	for (std::map<unsigned int, Client>::iterator it = connectedClientsList.begin(); it != connectedClientsList.end(); it++)
	{
		pack.clear();
		pack << COMMUNICATION_HEADER_SERVER_TO_CLIENT::DISCONNECT_SERVER << it->second.clientSalt << it->second.serverSalt;
		socketStatus = udpSocket.send(pack, it->second.ip, it->second.port);
	}

	return 0;
}