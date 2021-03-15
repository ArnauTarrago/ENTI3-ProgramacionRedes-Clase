#include "Interface.h"
#include <iostream>

void Interface::PrintScreen()
{
	ConsoleClear();
	ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::DARKRED);
	ConsoleXY(0, 0);
	std::cout << "State: " << gameState;

	ConsoleXY(40, 2);
	std::cout << "Next turns: " << "Player 1, Player 2, Player 3";

	ConsoleXY(55, 3);
	std::cout << "Turn: " << currentPlayer;

	// WE'LL HAVE TO CALCULATE THE SIZE OF THE PLAYER NAME AND POSITION IT ACCORDINGLY
	ConsoleXY(105, 0);
	std::cout << "50 | " << " Player 1";
	ConsoleXY(105, 1);
	std::cout << "47 | " << " Player 2";
	ConsoleXY(105, 2);
	std::cout << "83 | " << " Player 3";

	
	
}

void Interface::SetGameState(std::string _gameState)
{
	gameState = _gameState;
}

void Interface::SetCurrentPlayerTurn(std::string _currentPlayer)
{
	currentPlayer = _currentPlayer;
}

void Interface::SetNextTurn(std::string _names[])
{

}

void Interface::SetScore(std::string _score[])
{

}

void Interface::SetConnectedPlayers(std::string _connectedPlayers[])
{

}


