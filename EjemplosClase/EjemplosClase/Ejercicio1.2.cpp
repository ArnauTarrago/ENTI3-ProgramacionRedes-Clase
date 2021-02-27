#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <chrono>
#include <string>

bool HandleServer(sf::TcpSocket* sock)
{
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

	status = serverListener.accept(*sock);
	serverListener.close();
	if (status != sf::Socket::Done)
	{
		std::cout << "Error en accept\n";
		return false;
	}
	return true;
}

bool HandlePlayer(sf::TcpSocket* sock)
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Socket::Status status;
	std::cout << "Which IP do you want to connect to?: ";
	std::cin >> ip;

	std::cout << "Which port do you want to connect to?: ";
	std::cin >> port;

	status = sock->connect(ip,port);

	if (status != sf::Socket::Done)
	{
		std::cout << "Error connecting\n";
		return false;
	}
	std::cout << "Connected!\n";
	return true;
	
}

void Reception(sf::TcpSocket* sock)
{
	sf::Socket::Status status;
	while (true)
	{
		sf::Packet pack;
		status = sock->receive(pack);
		if (status == sf::Socket::Done)
		{
			std::string str;
			pack >> str;
			std::cout << str << std::endl;
		}
		else break;
	}
}

void Send(sf::TcpSocket* sock)
{
	sf::Socket::Status status;
	while (true)
	{
		std::string str;
		std::getline(std::cin, str);
		sf::Packet pack;
		pack << str;
		status = sock->send(pack);
		if (status == sf::Socket::Status::Disconnected || str == "exit")
		{
			break;
		}

		pack.clear();
	}
}

int main()
{
	std::string userType;
	std::string message;

	std::cout << "Are you a client 'c' or server 's'?: ";
	std::cin >> userType;
	sf::TcpSocket* sock = new sf::TcpSocket();
	bool okConnection = false;

	if (userType == "s")
	{
		okConnection = HandleServer(sock);
	}
	else if (userType == "c")
	{
		okConnection = HandlePlayer(sock);
	}	

	if (okConnection)
	{
		std::thread tSend(Send, sock);
		tSend.detach();

		Reception(sock);
	}

	sock->disconnect();
	delete sock;
	return 0;
}