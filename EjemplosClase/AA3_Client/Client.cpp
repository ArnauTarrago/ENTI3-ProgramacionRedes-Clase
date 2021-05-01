#include <string>
#include <thread>
#include <iostream>   
#include <vector>
#include <mutex>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>


sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
sf::IpAddress serverIP = "localhost";
unsigned short serverPort = 50000;
std::mutex mtx_messages;
std::vector<std::string> aMensajes;

void Receive(sf::IpAddress _serverIP, unsigned short _serverPort)
{
	while (true)
	{
		sf::Packet pack;
		std::string message;

		socketStatus = udpSocket.receive(pack, _serverIP, _serverPort);

		mtx_messages.lock();
		if (pack >> message)
		{
			aMensajes.push_back(message);
		}
		mtx_messages.unlock();
	}
}

int main()
{
	sf::Packet pack;

	std::cout << "Enter server IP: ";
	std::cin >> serverIP;
	std::cout << "Enter server port: ";
	std::cin >> serverPort;

	socketStatus = udpSocket.send(pack, serverIP, serverPort);

	std::thread tReceive(&Receive, serverIP, serverPort);
	tReceive.detach();

	if (socketStatus != sf::UdpSocket::Status::Done)
	{
		std::cout << "Connection failed." << std::endl;
	}
	else
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
						std::string aux = mensaje;
						pack << aux;			
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
