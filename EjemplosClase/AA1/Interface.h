#include <sstream>
#include <string>
#include <iostream>       
#include "ConsoleControl.h"
#pragma once

class Interface
{
public:
	struct InterfaceArea {
		const short int AREA_START_X = 0;
		const short int AREA_START_Y = 0;
		const short int AREA_WIDTH = 0;
		const short int AREA_HEIGHT = 0;
		const ConsoleColor AREA_COLOR_CHAR = LIGHTGREY;
		const ConsoleColor AREA_COLOR_BACK = BLACK;
		InterfaceArea(
			short int _AREA_START_X = 0,
			short int _AREA_START_Y = 0,
			short int _AREA_WIDTH = 0,
			short int _AREA_HEIGHT = 0,
			ConsoleColor _AREA_COLOR_CHAR = LIGHTGREY,
			ConsoleColor _AREA_COLOR_BACK = BLACK
		) : AREA_START_X(_AREA_START_X),
			AREA_START_Y(_AREA_START_Y),
			AREA_WIDTH(_AREA_WIDTH),
			AREA_HEIGHT(_AREA_HEIGHT),
			AREA_COLOR_CHAR(_AREA_COLOR_CHAR),
			AREA_COLOR_BACK(_AREA_COLOR_BACK) {};
		void Clear() {
			ConsoleSetColor(AREA_COLOR_CHAR, AREA_COLOR_BACK);
			for (size_t i = 0; i < AREA_HEIGHT; i++)
			{
				ConsoleXY(AREA_START_X, AREA_START_Y + i);
				for (size_t i = 0; i < AREA_WIDTH; i++)
				{
					cout << ' ';
				}
			}
			ConsoleXY(AREA_START_X, AREA_START_Y);
			ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
		}
		void SetText(const string* text, bool resetCursor = true) {
			SetText(text->c_str(), resetCursor);
		}
		void SetText(const char* text, bool resetCursor = true) {
			ConsoleSetColor(AREA_COLOR_CHAR, AREA_COLOR_BACK);
			ConsoleXY(AREA_START_X, AREA_START_Y);
			cout << text;
			if (resetCursor) {
				ConsoleXY(AREA_START_X, AREA_START_Y);
				ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
			}
		}
	};
	InterfaceArea messages = InterfaceArea(5, 5, 50, 20, WHITE, DARKGREY);
	InterfaceArea gamelist = InterfaceArea(60, 5, 50, 20, WHITE, DARKGREY);
	InterfaceArea commandArea = InterfaceArea(1, 28, 118, 1, BLACK, WHITE);
	Interface() {
		ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
		ConsoleClear();
		messages.Clear();
		gamelist.Clear();
		commandArea.Clear();
	}
	void UpdateClient(const Player* player) {
		switch (player->currentState)
		{
		case PLAYER_STATES::PLAYER_STATE_CONNECTING:
			break;
		case PLAYER_STATES::PLAYER_STATE_BROWSING:
			break;
		case PLAYER_STATES::PLAYER_STATE_LOADING:
			break;
		case PLAYER_STATES::PLAYER_STATE_SETUP:
			break;
		case PLAYER_STATES::PLAYER_STATE_INGAME:
			break;
		default:
			break;
		}
	}
	void UpdateServer(const BSS* server) {
		switch (server->currentState)
		{
		case SERVER_STATES::SERVER_STATE_CONNECTING:
			break;
		case SERVER_STATES::SERVER_STATE_MANAGING:
			break;
		default:
			break;
		}
	}
	void PrintScreen() {
		ConsoleClear();
		PrintMessages();
		gamelist.Clear();
		commandArea.Clear();
		commandArea.SetText(">", false);
		//ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::DARKRED);
		//ConsoleXY(0, 0);
		//std::cout << "State: " << gameState;

		//ConsoleXY(40, 2);
		//std::cout << "Next turns: " << "Player 1, Player 2, Player 3";

		//ConsoleXY(55, 3);
		//std::cout << "Turn: " << currentPlayer;

		//// WE'LL HAVE TO CALCULATE THE SIZE OF THE PLAYER NAME AND POSITION IT ACCORDINGLY
		//ConsoleXY(105, 0);
		//std::cout << "50 | " << " Player 1";
		//ConsoleXY(105, 1);
		//std::cout << "47 | " << " Player 2";
		//ConsoleXY(105, 2);
		//std::cout << "83 | " << " Player 3";

	}
	void SetGameState(std::string _gameState) {
		gameState = _gameState;
	}
	void SetCurrentPlayerTurn(std::string _currentPlayer) {
		currentPlayer = _currentPlayer;
	}
	void SetNextTurn(std::string _names[]) {

	}
	void SetScore(std::string _score[]) {
	}
	void SetConnectedPlayers(std::string _connectedPlayers[]) {

	}
	void PrintMessages() {
		messages.Clear();
		ConsoleSetColor(messages.AREA_COLOR_CHAR, messages.AREA_COLOR_BACK);
		int j = 0;
		for (int i = consoleMessages.size() - 1; i >= 0; i--)
		{
			ConsoleXY(messages.AREA_START_X, messages.AREA_START_Y + messages.AREA_HEIGHT - j - 1);
			cout << consoleMessages[i];
			j++;
			if (j >= messages.AREA_HEIGHT)
				break;
		}
		ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
	}
	void AddLine(const string message, vector<string>* messagelist, const int MAX_LENGTH) {
		string newMessage = string(message);
		vector<string> tempmessages;
		int j = 0;
		ostringstream stringstream;
		for (size_t i = 0; i < newMessage.size(); i++)
		{
			char tempchar = newMessage.at(i);
			if (tempchar == '\n' || j >= MAX_LENGTH) {
				//string temp = string(stringstream.str());
				tempmessages.push_back(stringstream.str());
				stringstream.str("");
				j = 0;
			}
			else {
				stringstream << newMessage.at(i);
				j++;
			}
		}
		//string temp = string(stringstream.str());
		tempmessages.push_back(stringstream.str());
		stringstream.str("");
		//reverse(tempmessages.begin(), tempmessages.end());
		messagelist->insert(messagelist->end(), tempmessages.begin(), tempmessages.end());
	}
	void AddMessage(const string message, bool print = false) {
		AddLine(message, &consoleMessages, messages.AREA_WIDTH);
		if (print)
			PrintMessages();
	}
	string GetLine(bool registerLine = true) {
		commandArea.Clear();
		commandArea.SetText(">", false);
		string temp;
		cin >> temp;
		if (registerLine) {
			AddMessage(temp);
			PrintScreen();
		}
		return temp;
	}
private:
	std::string gameState, currentPlayer;
	std::string names[6]; // CHANGE TO VECTORS
	std::string score[6]; // CHANGE TO VECTORS
	std::string connectedPlayers[6]; // CHANGE TO VECTORS
	vector<string> consoleMessages;
};
