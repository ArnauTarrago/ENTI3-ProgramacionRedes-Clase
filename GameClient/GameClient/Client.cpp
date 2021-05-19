#include <string>
#include <thread>
#include <iostream>   
#include <vector>
#include <mutex>
#include <sstream>
#include <Constants.h>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include "shared.h"
#include "timer.h"
#include <GameInfo.h>

#define SIZE 10.f
///TAMAÑO EN PX DE LA VENTANA
#define W_WINDOW_PX 800
#define H_WINDOW_PX 600

///TAMAÑO EN CELDAS DE LA VENTANA
#define W_WINDOW_TITLE 80
#define H_WINDOW_TITLE 60

///TAMAÑO EN CELDAS DE UNA ZONA DE MAPA
#define W_WINDOW_TITLE_PART 40
#define H_WINDOW_TITLE_PART 30

///NUMERO DE ENEMIGOS Y DE OBSTACULOS QUE TIENEN QUE GENERARSE POR ZONA
#define NUM_ENEMIES 5
#define NUM_OBSTACLES_2x2 10

CLIENT_STATUS clientStatus = CLIENT_STATUS::DISCONNECTED;
long long int serverSalt, clientSalt;
sf::IpAddress serverIP;
unsigned short serverPort;
unsigned int clientID;
sf::Vector2f playerPosition;

sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::mutex mtx_messages;
std::vector<std::string> aMensajes;

Timer helloTimer, inactivityTimer;

//struct CriticalPacket
//{
//	COMMUNICATION_HEADER_CLIENT_TO_SERVER header;
//};
//std::map<int, CriticalPacket> listMsgNonAck;
//int localPacketID = 0;
unsigned int localMoveID = 0;

#pragma region Operations
unsigned int GenerateMoveID()
{
	unsigned int auxMoveID = localMoveID;
	localMoveID++;
	if (localMoveID == numeric_limits<unsigned int>::max())
	{
		localMoveID = 0;
	}
	return auxMoveID;
}
#pragma endregion

#pragma region Validations
bool IsClientSaltValid(long long int auxClientSalt)
{
	return auxClientSalt == clientSalt;
}

bool IsServerSaltValid(long long int auxServerSalt)
{
	return auxServerSalt == serverSalt;
}

bool AreSaltsValid(long long int auxServerSalt, long long int auxClientSalt)
{
	return IsServerSaltValid(auxServerSalt) && IsClientSaltValid(auxClientSalt);
}

int GetChallengeResponse(int challenge)
{
	return challenge + 1;
}
#pragma endregion

#pragma region Threads

void Receive(sf::IpAddress _serverIP, unsigned short _serverPort)
{
	COMMUNICATION_HEADER_SERVER_TO_CLIENT auxCommHeader;
	int msg;

	while (clientStatus != CLIENT_STATUS::DISCONNECTED)
	{
		sf::Packet pack;
		std::string message;

		socketStatus = udpSocket.receive(pack, _serverIP, _serverPort);

		if (pack >> msg)
		{
			long long int auxClientSalt, auxServerSalt;
			auxCommHeader = (COMMUNICATION_HEADER_SERVER_TO_CLIENT)msg;

			// WE RESET THE X SECONDS TIMER USED FOR CHECKING FOR INACTIVITY
			if (clientStatus == CLIENT_STATUS::CONNECTING) helloTimer.start();
			inactivityTimer.start();

			pack >> auxClientSalt >> auxServerSalt;

			switch (auxCommHeader)
			{
			case CHALLENGE:
				int challenge, challengeResponse;
				pack >> challenge;
				if (IsClientSaltValid(auxClientSalt))
				{
					std::cout << "Server has sent back a CHALLENGE with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					serverSalt = auxServerSalt;
					pack.clear();
					challengeResponse = GetChallengeResponse(challenge);
					pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::CHALLENGE_R << auxClientSalt << auxServerSalt << challengeResponse;
					socketStatus = udpSocket.send(pack, _serverIP, _serverPort);
				}				
				break;
			case WELCOME:
				if (AreSaltsValid(auxServerSalt, auxClientSalt))
				{
					int numberOfConnectedClients;

					pack >> clientID >> numberOfConnectedClients;

					for (size_t i = 0; i < numberOfConnectedClients; i++)
					{
						// TODO: UNPACK AND STORE OTHER CLIENTS' POSITIONS
						// pack >> connectedClientID >> positionX >> positionY
					}
					std::cout << "Server has sent back a WELCOME with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
					clientStatus = CLIENT_STATUS::CONNECTED;
				}
				break;
			case CHAT_SERVER_TO_CLIENT:
			{
				sf::Uint32 temp;
				sf::Uint32 temp2;
				pack >> temp >> temp2 >> message;
				if (AreSaltsValid(auxServerSalt, auxClientSalt))
				{
					sf::IpAddress ip = sf::IpAddress(temp);
					std::cout << "Server has sent back a CHAT with salt " << auxClientSalt << "/" << auxServerSalt << " and message: " << message << std::endl;
					stringstream ss;
					ss << "[" << ip.toString() << ":" << temp2 << "]: " << message;
					mtx_messages.lock();
					aMensajes.push_back(ss.str());
					mtx_messages.unlock();
				}
				
			}
			break;
			case DISCONNECT_CLIENT_HAS_DISCONNECTED:
			{
				sf::Uint32 temp, tempPacketID;
				unsigned short port;
				pack >> tempPacketID >> temp >> port;
				if (AreSaltsValid(auxServerSalt, auxClientSalt))
				{
					pack.clear();
					//packetID = tempPacketID;
					pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::ACKNOWLEDGE << auxClientSalt << auxServerSalt << clientID << tempPacketID;
					socketStatus = udpSocket.send(pack, serverIP, serverPort);

					sf::IpAddress ip = sf::IpAddress(temp);

					std::cout << "Client with IP " << ip.toString() << " and port " << port << " has disconnected from the server." << std::endl;
					stringstream ss;
					ss << "[" << ip.toString() << ":" << ip << "]: " << "Client with IP " << ip.toString() << " and port " << port << " has disconnected from the server.";
					mtx_messages.lock();
					aMensajes.push_back(ss.str());
					mtx_messages.unlock();
				}

			}
			break;
			case NEW_CLIENT:
				sf::Uint32 temp, tempPacketID;
				unsigned long packetID;
				unsigned short port;
				Cell clientPosition;

				pack >> tempPacketID >> temp >> port >> clientPosition.x >> clientPosition.y;
				if (AreSaltsValid(auxServerSalt, auxClientSalt))
				{
					pack.clear();
					//packetID = tempPacketID;
					pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::ACKNOWLEDGE << auxClientSalt << auxServerSalt << clientID << tempPacketID;
					socketStatus = udpSocket.send(pack, serverIP, serverPort);

					sf::IpAddress ip = sf::IpAddress(temp);

					std::cout << "Client with IP " << ip.toString() << " and port " << port << " has connected to the server in position: (" << clientPosition.x << "," << clientPosition.y  << ")." << std::endl;
					stringstream ss;
					ss << "[" << ip.toString() << ":" << port << "]: " << "Client with IP " << ip.toString() << " and port " << port << " has connected to the server in position: (" << clientPosition.x << "," << clientPosition.y << ").";
					mtx_messages.lock();
					aMensajes.push_back(ss.str());
					mtx_messages.unlock();
				}
				break;
			case DISCONNECT_SERVER:
				std::cout << "Server disconnected, closing..." << std::endl;
				clientStatus = CLIENT_STATUS::DISCONNECTED;
				break;
			case OKMOVE:
				sf::Uint32 tempMoveID;
				unsigned long moveID;
				float playerPositionX, playerPositionY;

				pack >> tempMoveID >> playerPositionX >> playerPositionY;
				if (AreSaltsValid(auxServerSalt, auxClientSalt))
				{
					playerPosition.x = playerPositionX;
					playerPosition.y = playerPositionY;
				}
				break;
			default:
				break;
			}

		}
		
	}
}

void TimerCheck()
{
	sf::Packet pack;

	helloTimer.start();
	inactivityTimer.start();
	while (clientStatus != CLIENT_STATUS::DISCONNECTED)
	{
		if (clientStatus == CLIENT_STATUS::CONNECTING)
		{
			if (helloTimer.elapsedSeconds() > TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_WHILE_CONNECTED_IN_SECONDS)
			{
				std::cout << "Resent HELLO to server with salt: " << clientSalt << std::endl;
				pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::HELLO << clientSalt;
				socketStatus = udpSocket.send(pack, serverIP, serverPort);

				helloTimer.start();
			}
			if (inactivityTimer.elapsedSeconds() > TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_DURING_CONNECTION_IN_SECONDS)
			{
				std::cout << "30 seconds have passed since the server responded, cancelling connection..." << std::endl;
				helloTimer.stop();
				inactivityTimer.stop();
				clientStatus = CLIENT_STATUS::DISCONNECTED;
			}
		}
		else if (clientStatus == CLIENT_STATUS::CONNECTED)
		{
			if (inactivityTimer.elapsedSeconds() > TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_WHILE_CONNECTED_IN_SECONDS)
			{
				std::cout << "60 seconds have passed since the server responded, disconnecting from server..." << std::endl;
				helloTimer.stop();
				inactivityTimer.stop();
				clientStatus = CLIENT_STATUS::DISCONNECTED;
			}
		}
	}

}
#pragma endregion


void DrawGraphics()
{
	sf::Packet pack;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = " >";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	sf::RenderWindow _window(sf::VideoMode(800, 600), "Ventanita");
	sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
	shape.setOutlineColor(sf::Color::Black);
	shape.setOutlineThickness(2.f);

	while (window.isOpen() && _window.isOpen())
	{
		if (clientStatus == CLIENT_STATUS::DISCONNECTED)
		{
			window.close();
		}

		// CHAT SCREEN
		sf::Event evento;

		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					mtx_messages.lock();
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					mtx_messages.unlock();
					pack.clear();
					std::string chatMessage = mensaje;
					if (chatMessage == "exit" || chatMessage == ">exit")
					{
						pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::DISCONNECT_CLIENT << clientSalt << serverSalt << clientID;
						std::cout << "Disconnecting from server." << std::endl;
						clientStatus = CLIENT_STATUS::DISCONNECTED;
					}
					else
					{
						pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::CHAT_CLIENT_TO_SERVER << clientSalt << serverSalt << clientID << chatMessage;
					}

					socketStatus = udpSocket.send(pack, serverIP, serverPort);
					pack.clear();
					mensaje = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
		window.draw(separator);
		mtx_messages.lock();
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		mtx_messages.unlock();
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();

		// GAME SCREEN
		sf::Event event;
		sf::Vector2f position;
		position.x = 0; position.y = 0;
		shape.setFillColor(sf::Color::Blue);
		shape.setFillColor(sf::Color(0, 0, 255, 255));
		shape.setPosition(sf::Vector2f(playerPosition.x* SIZE, playerPosition.y* SIZE));
		_window.draw(shape);

		position.x = W_WINDOW_TITLE - 1; position.y = H_WINDOW_TITLE - 1;
		shape.setFillColor(sf::Color::Green);
		shape.setFillColor(sf::Color(255, 255, 0, 255));
		shape.setPosition(sf::Vector2f(position.x* SIZE, position.y* SIZE));
		_window.draw(shape);

		_window.display();
		bool playerMoved = false;

		while (_window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				_window.close();
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape)
				{
					_window.close();
				}
				if (event.key.code == sf::Keyboard::Left)
				{
					std::cout << "LEFT\n";					
				}
				else if (event.key.code == sf::Keyboard::Up)
				{
					std::cout << "UP\n";
				}
				else if (event.key.code == sf::Keyboard::Right)
				{
					std::cout << "RIGHT\n";
					pack.clear();
					pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::MOVE << clientSalt << serverSalt << clientID << GenerateMoveID() << playerPosition.x + 1 << playerPosition.y;
					socketStatus = udpSocket.send(pack, serverIP, serverPort);
				}
				else if (event.key.code == sf::Keyboard::Down)
				{
					std::cout << "DOWN\n";
				}
				break;
			}
		}
		_window.clear();
		for (int i = 0; i < W_WINDOW_TITLE; i++)
		{
			for (int j = 0; j < H_WINDOW_TITLE; j++)
			{

				shape.setFillColor(sf::Color(90, 90, 90, 255));


				shape.setPosition(sf::Vector2f(i * SIZE, j * SIZE));
				_window.draw(shape);
			}
		}





	}

	
}

int main()
{
	sf::Packet pack;

	std::cout << "Enter server IP: ";
	std::cin >> serverIP;
	std::cout << "Enter server port: ";
	std::cin >> serverPort;

	clientSalt = Utilities::GenerateSalt();
	std::cout << "Generated salt: " << clientSalt << endl;

	pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::HELLO << clientSalt;
	clientStatus = CLIENT_STATUS::CONNECTING;

	socketStatus = udpSocket.send(pack, serverIP, serverPort);

	std::thread tReceive(&Receive, serverIP, serverPort);
	std::thread tSend(&TimerCheck);

	playerPosition.x = 0;
	playerPosition.y = 0;

	std::cout << "Awaiting connection with server, please wait..." << std::endl;
	while (clientStatus == CLIENT_STATUS::CONNECTING)
	{

	}

	if (clientStatus == CLIENT_STATUS::DISCONNECTED)
	{
		std::cout << "Connection failed." << std::endl;
	}

	else if (clientStatus == CLIENT_STATUS::CONNECTED)
	{
		std::cout << "Connection succesful." << std::endl;

		DrawGraphics();
	}
	tReceive.join();
	tSend.join();

	system("pause");
	return 0;
}
