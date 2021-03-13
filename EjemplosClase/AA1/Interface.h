#include <string>         
#include "ConsoleControl.h"
#pragma once

class Interface
{
public:	
	void PrintScreen();
	void SetGameState(std::string _gameState);
	void SetCurrentPlayerTurn(std::string _currentPlayer);
	void SetNextTurn(std::string _names[]);
	void SetScore(std::string _score[]); // SCORE AND PLAYERS WILL BE LINKED
	void SetConnectedPlayers(std::string _connectedPlayers[]); // SCORE AND PLAYERS WILL BE LINKED
private:
	std::string gameState, currentPlayer;
	std::string names[6]; // CHANGE TO VECTORS
	std::string score[6]; // CHANGE TO VECTORS
	std::string connectedPlayers[6]; // CHANGE TO VECTORS
};


