#include <string>
#include <thread>
#include <iostream>   
#include <vector>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>


sf::UdpSocket udpSocket;
sf::UdpSocket::Status socketStatus = sf::UdpSocket::Status::NotReady;
sf::IpAddress serverIP = "localhost";
unsigned short serverPort = 50000;



void Send(sf::IpAddress _serverIP, unsigned short _serverPort)
{
	while (true)
	{
		sf::Packet pack;
		std::string message;

		std::getline(std::cin, message);
		pack << message;

		socketStatus = udpSocket.send(pack, _serverIP, _serverPort);
	}
}

void LoadChatWindow()
{

}

//void UpdateChatWindow()
//{
//	std::vector<std::string> aMensajes;
//
//	sf::Vector2i screenDimensions(800, 600);
//
//	sf::RenderWindow window;
//	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");
//
//	sf::Font font;
//	if (!font.loadFromFile("courbd.ttf"))
//	{
//		std::cout << "Can't load the font file" << std::endl;
//	}
//
//	sf::String mensaje = " >";
//
//	sf::Text chattingText(mensaje, font, 14);
//	chattingText.setFillColor(sf::Color(0, 160, 0));
//	chattingText.setStyle(sf::Text::Bold);
//
//
//	sf::Text text(mensaje, font, 14);
//	text.setFillColor(sf::Color(0, 160, 0));
//	text.setStyle(sf::Text::Bold);
//	text.setPosition(0, 560);
//
//	sf::RectangleShape separator(sf::Vector2f(800, 5));
//	separator.setFillColor(sf::Color(200, 200, 200, 255));
//	separator.setPosition(0, 550);
//
//	while (window.isOpen())
//	{
//		sf::Event evento;
//		while (window.pollEvent(evento))
//		{
//			switch (evento.type)
//			{
//			case sf::Event::Closed:
//				window.close();
//				break;
//			case sf::Event::KeyPressed:
//				if (evento.key.code == sf::Keyboard::Escape)
//					window.close();
//				else if (evento.key.code == sf::Keyboard::Return)
//				{
//					aMensajes.push_back(mensaje);
//					if (aMensajes.size() > 25)
//					{
//						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
//					}
//					mensaje = ">";
//				}
//				break;
//			case sf::Event::TextEntered:
//				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
//					mensaje += (char)evento.text.unicode;
//				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
//					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
//				break;
//			}
//		}
//		window.draw(separator);
//		for (size_t i = 0; i < aMensajes.size(); i++)
//		{
//			std::string chatting = aMensajes[i];
//			chattingText.setPosition(sf::Vector2f(0, 20 * i));
//			chattingText.setString(chatting);
//			window.draw(chattingText);
//		}
//		std::string mensaje_ = mensaje + "_";
//		text.setString(mensaje_);
//		window.draw(text);
//
//
//		window.display();
//		window.clear();
//	}
//}

int main()
{
	sf::Packet pack;

	std::cout << "Enter server IP: ";
	std::cin >> serverIP;
	std::cout << "Enter server port: ";
	std::cin >> serverPort;

	socketStatus = udpSocket.send(pack, serverIP, serverPort);

	std::thread tSend(&Send, serverIP, serverPort);
	tSend.detach();

	if (socketStatus != sf::UdpSocket::Status::Done)
	{
		std::cout << "Connection failed." << std::endl;
	}
	else
	{
		std::cout << "Connection succesful." << std::endl;

		while (true)
		{
			std::string message;

			socketStatus = udpSocket.receive(pack, serverIP, serverPort);



			if (pack >> message)
			{
				std::cout << message << std::endl;
			}

			// TODO: HANDLE THE TYPE OF MESSAGE SENT BY THE SERVER
		}
	}	

	system("pause");
	return 0;
}
