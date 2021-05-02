#include <string>
#include <thread>
#include <iostream>   
#include <vector>
#include <mutex>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include "../shared.h"
#include "../timer.h"

CLIENT_STATUS clientStatus = CLIENT_STATUS::DISCONNECTED;
long long int serverSalt, clientSalt;
sf::IpAddress serverIP;
unsigned short serverPort;

sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
std::mutex mtx_messages;
std::vector<std::string> aMensajes;

Timer helloTimer, inactivityTimer;

void Receive(sf::IpAddress _serverIP, unsigned short _serverPort)
{
	COMMUNICATION_HEADER_SERVER_TO_CLIENT auxCommHeader;
	int msg;

	while (true)
	{
		sf::Packet pack;
		std::string message;

		socketStatus = udpSocket.receive(pack, _serverIP, _serverPort);

		mtx_messages.lock();
		if (pack >> msg)
		{
			long long int auxClientSalt, auxServerSalt;
			auxCommHeader = (COMMUNICATION_HEADER_SERVER_TO_CLIENT)msg;
			// WE RESET THE X SECONDS TIMER USED FOR CHECKING FOR INACTIVITY
			helloTimer.start();
			inactivityTimer.start();
			switch (auxCommHeader)
			{
			case CHALLENGE:

				pack >> auxClientSalt >> auxServerSalt;
				std::cout << "Server has sent back a CHALLENGE with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
				pack.clear();
				pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::CHALLENGE_R << auxClientSalt << auxServerSalt;
				socketStatus = udpSocket.send(pack, _serverIP, _serverPort);
				break;
			case WELCOME:
				pack >> auxClientSalt >> auxServerSalt;
				std::cout << "Server has sent back a WELCOME with salt: " << auxClientSalt << "/" << auxServerSalt << std::endl;
				clientStatus = CLIENT_STATUS::CONNECTED;
				break;
			case CHAT_SERVER_TO_CLIENT:
				pack >> message;
				aMensajes.push_back(message);
				break;
			default:
				break;
			}
			
		}
		mtx_messages.unlock();
	}
}

void Send()
{
	sf::Packet pack;

	helloTimer.start();
	inactivityTimer.start();
	while (clientStatus != CLIENT_STATUS::DISCONNECTED)
	{
		if (helloTimer.elapsedSeconds() > 5.0)
		{
			if (clientStatus == CLIENT_STATUS::CONNECTING)
			{
				std::cout << "Resent HELLO to server with salt: " << clientSalt << std::endl;
				pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::HELLO << clientSalt;
				socketStatus = udpSocket.send(pack, serverIP, serverPort);
			}
			helloTimer.start();					
		}
		if (inactivityTimer.elapsedSeconds() > 30)
		{
			std::cout << "30 seconds have passed since the server responded, disconnecting..." << std::endl;
			helloTimer.stop();
			inactivityTimer.stop();
			clientStatus = CLIENT_STATUS::DISCONNECTED;
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
	tReceive.detach();

	std::thread tSend(&Send);
	tSend.detach();

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

		while (window.isOpen())
		{
			if (clientStatus == CLIENT_STATUS::DISCONNECTED)
			{
				window.close();
			}

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
						pack << COMMUNICATION_HEADER_CLIENT_TO_SERVER::CHAT_CLIENT_TO_SERVER << clientSalt << chatMessage;
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
		}
	}	

	system("pause");
	return 0;
}
