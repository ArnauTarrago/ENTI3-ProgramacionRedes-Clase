#include <sstream>
#include <string>
#include <iostream>      
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
		string innerText;
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
			PrintText();
		}
		void SetText(const string* text, bool resetCursor = true) {
			SetText(text->c_str(), resetCursor);
		}
		void SetText(const char* text, bool resetCursor = true) {
			innerText = text;
			PrintText(resetCursor);
		}
		void PrintText(bool resetCursor = true) {
			ConsoleSetColor(AREA_COLOR_CHAR, AREA_COLOR_BACK);
			ConsoleXY(AREA_START_X, AREA_START_Y);
			cout << innerText;
			if (resetCursor) {
				ConsoleXY(AREA_START_X, AREA_START_Y);
				ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
			}
		}
	};
	InterfaceArea messagesTitle = InterfaceArea(1, 6, 58, 1, DARKGREY, LIGHTGREY);
	InterfaceArea messages = InterfaceArea(1, 7, 58, 20, WHITE, DARKGREY);
	InterfaceArea gamelistTitle = InterfaceArea(61, 6, 58, 1, DARKGREY, LIGHTGREY);
	InterfaceArea gamelist = InterfaceArea(61, 7, 58, 20, WHITE, DARKGREY);
	InterfaceArea commandArea = InterfaceArea(1, 28, 118, 1, BLACK, WHITE);
	Interface() {
		ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
		ConsoleClear();
		messagesTitle.Clear();
		messages.Clear();
		gamelist.Clear();
		commandArea.Clear();
		gamelistTitle.Clear();
		messagesTitle.SetText("Console:");
	}
	void ResetCursor() {
		ConsoleXY(commandArea.AREA_START_X + 1, commandArea.AREA_START_Y + 1);
		ConsoleSetColor(commandArea.AREA_COLOR_CHAR, commandArea.AREA_COLOR_BACK);
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
		messagesTitle.Clear();
		gamelistTitle.Clear();
		commandArea.SetText(">", false);
		ResetCursor();
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

	void PrintGamelist(const vector<GameSessionSend> games) {
		gamelist.Clear();
		gamelistTitle.Clear();
		gamelistTitle.SetText("Current games:");
		ConsoleSetColor(gamelist.AREA_COLOR_CHAR, gamelist.AREA_COLOR_BACK);
		for (size_t i = 0; i < games.size(); i++)
		{
			ConsoleXY(gamelist.AREA_START_X, gamelist.AREA_START_Y + i);
			cout << ' ' << get<0>(games[i]);
			ConsoleXY(gamelist.AREA_START_X + gamelist.AREA_WIDTH - 10, gamelist.AREA_START_Y + i);
			cout << "| ";
			if (get<3>(games[i])) {
				ConsoleSetColor(RED, gamelist.AREA_COLOR_BACK);
				cout << 'x';
			}
			else {
				ConsoleSetColor(GREEN, gamelist.AREA_COLOR_BACK);
				cout << 'o';
			}
			ConsoleSetColor(gamelist.AREA_COLOR_CHAR, gamelist.AREA_COLOR_BACK);
			cout << " | " << get<1>(games[i]) << "/" << get<2>(games[i]);
		}
		ResetCursor();
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
			ConsoleSetColor(get<0>(consoleMessages[i]), messages.AREA_COLOR_BACK);
			cout << get<1>(consoleMessages[i]);
			j++;
			if (j >= messages.AREA_HEIGHT)
				break;
		}
		ResetCursor();
	}
	void AddLine(const string message, vector<tuple<ConsoleColor, string>>* messagelist, const int MAX_LENGTH, const ConsoleColor color) {
		string newMessage = string(message);
		vector<tuple<ConsoleColor, string>> tempmessages;
		int j = 0;
		ostringstream stringstream;
		for (size_t i = 0; i < newMessage.size(); i++)
		{
			char tempchar = newMessage.at(i);
			if (tempchar == '\n' || j >= MAX_LENGTH) {
				//string temp = string(stringstream.str());
				tempmessages.push_back(make_tuple(color, stringstream.str()));
				stringstream.str("");
				j = 0;
			}
			else {
				stringstream << newMessage.at(i);
				j++;
			}
		}
		//string temp = string(stringstream.str());
		tempmessages.push_back(make_tuple(color, stringstream.str()));
		stringstream.str("");
		//reverse(tempmessages.begin(), tempmessages.end());
		messagelist->insert(messagelist->end(), tempmessages.begin(), tempmessages.end());
	}
	void AddMessage(const string message, ConsoleColor color = WHITE, bool print = true) {
		AddLine(message, &consoleMessages, messages.AREA_WIDTH, color);
		if (print)
			PrintMessages();
	}
	void AddConnection(const TcpSocket* temp, bool ok) {
		ostringstream stringstream;
		if (ok) {
			stringstream << "Connection: " << &temp << endl;
			stringstream << "	Ip : " << temp->getRemoteAddress() << endl;
			stringstream << "	Port : " << temp->getRemotePort() << endl;
			AddMessage(stringstream.str(), GREEN);
		}
		else {
			stringstream << "Connection lost: " << &temp << endl;
			stringstream << "	Ip : " << temp->getRemoteAddress() << endl;
			stringstream << "	Port : " << temp->getRemotePort() << endl;
			AddMessage(stringstream.str(), RED);
		}
	}

	string GetInput_String(bool registerLine = true) {
		commandArea.Clear();
		commandArea.SetText(">", false);
		string temp;
		cin >> temp;
		if (registerLine) {
			AddMessage(temp);
			PrintScreen();
		}
		ResetCursor();
		return temp;
	}
	char GetInput_Char(bool registerLine = true) {
		commandArea.Clear();
		commandArea.SetText(">", false);
		char temp;
		cin >> temp;
		if (registerLine) {
			stringstream ss;
			ss << temp;
			AddMessage(ss.str());
			PrintScreen();
		}
		ResetCursor();
		return temp;
	}
	int GetInput_Int(bool registerLine = true) {
		commandArea.Clear();
		commandArea.SetText(">", false);
		int temp;
		cin >> temp;
		if (registerLine) {
			stringstream ss;
			ss << temp;
			AddMessage(ss.str());
			PrintScreen();
		}
		ResetCursor();
		return temp;
	}

private:
	std::string gameState, currentPlayer;
	std::string names[6]; // CHANGE TO VECTORS
	std::string score[6]; // CHANGE TO VECTORS
	std::string connectedPlayers[6]; // CHANGE TO VECTORS
	vector<tuple<ConsoleColor, string>> consoleMessages;
};
