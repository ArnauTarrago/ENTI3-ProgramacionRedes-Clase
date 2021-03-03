#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <string>

#define MAXPLAYERS 3

#include "shared.h"
#include "BSS.h"
#include "Player.h"

void HandlePlayer(sf::TcpSocket* sock)
{
	std::vector<sf::TcpSocket*> peerList = std::vector<sf::TcpSocket*>();
	unsigned short localPort = sock->getLocalPort();
	sf::Socket::Status status;
	sf::Packet pack;
	int numberOfMaxPlayers;
	status = sock->receive(pack);

	if (status == sf::Socket::Done)
	{
		pack >> numberOfMaxPlayers;
	}

	// RECIBIMOS MENSAJES DEL SOCKET 0 (DEL SERVIDOR)
	while (true)
	{
		status = sock->receive(pack);
		if (status == sf::Socket::Done)
		{
			int intIPAddress;
			sf::IpAddress ipAddress;
			unsigned short port;
			if (pack >> intIPAddress >> port)
			{
				ipAddress = sf::IpAddress(intIPAddress);

				// NOS CONECTAMOS CON EL PEER UTILIZANDO LA IP Y SOCKET
				sf::TcpSocket* auxSocket = new sf::TcpSocket();
				sf::Socket::Status auxStatus;
				auxSocket->connect(ipAddress, port);

				peerList.push_back(auxSocket);

			}
		}
		else if (status == sf::Socket::Disconnected)
		{
			sock->disconnect();
			break;
		}
	}



	// COMO SABE EL CLIENTE QUE NO DEBE/DEBE EMPEZAR LA PARTIDA?


}

bool InitializeBSS(sf::TcpSocket* sock)
{
	std::vector<Peer> peerList = std::vector<Peer>();

	int port;
	sf::Socket::Status status;
	sf::TcpListener serverListener;
	std::cout << "Which port do you want to open?: ";
	std::cin >> port;

	status = serverListener.listen(port);

	if (status != sf::Socket::Done)
	{
		std::cout << "Error en listener\n";
		return false;
	}

	sf::IpAddress ip = sf::IpAddress::LocalHost;
	std::cout << ip.getLocalAddress() << ":" << std::to_string(port) << std::endl;

	while (peerList.size() < 4) {
		sf::TcpSocket* temp = new sf::TcpSocket();
		if (serverListener.accept(*temp) != sf::Socket::Done)
		{
			std::cout << "Conexión no aceptada" << std::endl;
		}
		else {
			std::cout << "Conexión establecida con : " << temp << std::endl;
			if (peerList.size() > 0)
			{
				// Informar al socket de la información de la gente en la peerList
				sf::Packet pack;
				for (size_t i = 0; i < peerList.size(); i++)
				{
					pack << peerList[i].ip.toInteger() << peerList[i].port /* << 4 */;
					status = sock->send(pack);
					if (status == sf::Socket::Status::Disconnected)
					{
						std::cout << "Conexión perdida con : " << temp << std::endl;
					}
					pack.clear();
				}
			}
			peerList.push_back(Peer(temp->getRemoteAddress(), temp->getRemotePort()));
			temp->disconnect();
		}
	}

	serverListener.close();

	return true;
}

bool ConnectPlayerToBSS(sf::TcpSocket* sock)
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Socket::Status status;
	std::cout << "Which server IP do you want to connect to?: ";
	std::cin >> ip;

	std::cout << "Which server port do you want to connect to?: ";
	std::cin >> port;

	status = sock->connect(ip, port);

	if (status != sf::Socket::Done)
	{
		std::cout << "Error connecting\n";
		return false;
	}
	std::cout << "Connected!\n";
	return true;
}

int main()
{
	char userType;

	std::cout << "Are you a client 'c' or server 's'?: ";
	std::cin >> userType;
	sf::TcpSocket* sock_0 = new sf::TcpSocket();
	bool okConnection = false;

	if (userType == 's')
	{
		BSS bss;
	}
	else if (userType == 'c')
	{
		Player player;
	}

	return 0;
}