#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <chrono>
#include <string>

std::mutex mtxPlayers;

void PrintPlayerList(std::vector<std::string>* playerList)
{
	while (true)
	{
		mtxPlayers.lock();
		for (size_t i = 0; i < playerList->size(); i++)
		{
			std::cout << playerList->at(i) << std::endl;
		}
		mtxPlayers.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	
}

void DealWithClient(std::string auxName, std::vector<std::string>* playerList)
{
	
	for (size_t i = 0; i < 10; i++)
	{
		std::cout << auxName << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	mtxPlayers.lock();
	for (size_t i = 0; i < playerList->size(); i++)
	{
		if (playerList->at(i) == auxName)
		{
			playerList->erase(playerList->begin() + i);
			break;
		}
	}
	mtxPlayers.unlock();
}


//int main()
//{	
//	std::vector<std::string> playerList;
//	std::string auxName = "";
//	std::thread tAll(PrintPlayerList, &playerList);
//	tAll.detach();
//
//	while (auxName != "exit")
//	{
//		std::cout << "Entra un nombre de jugador: ";
//		std::cin >> auxName;
//
//		mtxPlayers.lock();
//		playerList.push_back(auxName);
//		mtxPlayers.unlock();
//
//		std::thread t(DealWithClient,auxName,&playerList);
//		t.detach();
//	}
//	return 0;
//}