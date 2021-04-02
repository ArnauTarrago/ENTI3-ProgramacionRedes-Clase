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
		ConsoleColor AREA_COLOR_CHAR = LIGHTGREY;
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
	InterfaceArea statusTitle = InterfaceArea(1, 1, 10, 1, DARKGREY, LIGHTGREY);
	InterfaceArea statusType = InterfaceArea(1, 2, 10, 1, WHITE, DARKGREY);
	InterfaceArea statusConnection = InterfaceArea(1, 3, 10, 1, WHITE, DARKGREY);
	InterfaceArea statusGame = InterfaceArea(1, 4, 10, 1, WHITE, DARKGREY);


	InterfaceArea playersTitle = InterfaceArea(12, 1, 30, 1, DARKGREY, LIGHTGREY);
	InterfaceArea playersP0 = InterfaceArea(12, 2, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP1 = InterfaceArea(12, 3, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP2 = InterfaceArea(12, 4, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP3 = InterfaceArea(22, 2, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP4 = InterfaceArea(22, 3, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP5 = InterfaceArea(22, 4, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP6 = InterfaceArea(32, 2, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP7 = InterfaceArea(32, 3, 10, 1, WHITE, DARKGREY);
	InterfaceArea playersP8 = InterfaceArea(32, 4, 10, 1, WHITE, DARKGREY);

	InterfaceArea categoriesTitle = InterfaceArea(61, 1, 58, 1, DARKGREY, LIGHTGREY);
	InterfaceArea categories = InterfaceArea(61, 2, 58, 3, WHITE, DARKGREY);

	InterfaceArea messagesTitle = InterfaceArea(1, 6, 58, 1, DARKGREY, LIGHTGREY);
	InterfaceArea messages = InterfaceArea(1, 7, 58, 20, WHITE, DARKGREY);

	InterfaceArea gamelistTitle = InterfaceArea(61, 6, 58, 1, DARKGREY, LIGHTGREY);
	InterfaceArea gamelist = InterfaceArea(61, 7, 58, 20, WHITE, DARKGREY);

	InterfaceArea commandArea = InterfaceArea(1, 28, 118, 1, BLACK, WHITE);
	Interface() {
		ConsoleSetColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
		messagesTitle.SetText("Console:");
		statusTitle.SetText("- Status -");
		playersTitle.SetText("Players:");
		PrintScreen();
	}
	void ResetCursor() {
		ConsoleXY(commandArea.AREA_START_X + 1, commandArea.AREA_START_Y + 1);
		ConsoleSetColor(commandArea.AREA_COLOR_CHAR, commandArea.AREA_COLOR_BACK);
	}
	void UpdateClient(Player* player) {
		statusType.SetText(PROGRAMTYPE_STRINGS[programtype].c_str());
		statusType.Clear();
		statusConnection.SetText(PLAYER_STATES_STRINGS[player->currentState].c_str());
		statusConnection.Clear();
		statusGame.SetText(PLAYER_STATES_INGAME_STRINGS[player->currentStateIngame].c_str());
		statusGame.Clear();
		switch (player->currentState)
		{
		case PLAYER_STATES::PLAYER_STATE_CONNECTING:
			break;
		case PLAYER_STATES::PLAYER_STATE_BROWSING:
			break;
		case PLAYER_STATES::PLAYER_STATE_LOADING:
			break;
		case PLAYER_STATES::PLAYER_STATE_SETUP:
			playersTitle.SetText("Players");
			switch (player->PlayerID)
			{
			case 1:
				playersP1.AREA_COLOR_CHAR = GREEN;
				playersP1.SetText("P1:");
				break;
			case 2:
				playersP2.AREA_COLOR_CHAR = GREEN;
				playersP2.SetText("P2:");
				break;
			case 3:
				playersP3.AREA_COLOR_CHAR = GREEN;
				playersP3.SetText("P3:");
				break;
			case 4:
				playersP4.AREA_COLOR_CHAR = GREEN;
				playersP4.SetText("P4:");
				break;
			case 5:
				playersP5.AREA_COLOR_CHAR = GREEN;
				playersP5.SetText("P5:");
				break;
			case 6:
				playersP6.AREA_COLOR_CHAR = GREEN;
				playersP6.SetText("P6:");
				break;
			case 7:
				playersP7.AREA_COLOR_CHAR = GREEN;
				playersP7.SetText("P7:");
				break;
			case 8:
				playersP8.AREA_COLOR_CHAR = GREEN;
				playersP8.SetText("P8:");
				break;
			default:
				playersP0.AREA_COLOR_CHAR = GREEN;
				playersP0.SetText("P0:");
				break;
			}
			playersTitle.Clear();
			playersP0.Clear();
			playersP1.Clear();
			playersP2.Clear();
			playersP3.Clear();
			playersP4.Clear();
			playersP5.Clear();
			playersP6.Clear();
			playersP7.Clear();
			playersP8.Clear();
			categoriesTitle.SetText("Category card count:");
			categoriesTitle.Clear();
			categories.Clear();
			break;
		case PLAYER_STATES::PLAYER_STATE_INGAME:
			gamelistTitle.SetText("Current cards:");
			gamelistTitle.Clear();
			PrintHand(player->hands.at(player->PlayerID)->hand);

			if (player->hands.size() > 0) {
				playersP0.SetText(("P0:" + to_string(player->hands[0]->points)).c_str());
			}
			if (player->hands.size() > 1) {
				playersP1.SetText(("P1:" + to_string(player->hands[1]->points)).c_str());
			}
			if (player->hands.size() > 2) {
				playersP2.SetText(("P2:" + to_string(player->hands[2]->points)).c_str());
			}
			if (player->hands.size() > 3) {
				playersP3.SetText(("P3:" + to_string(player->hands[3]->points)).c_str());
			}
			if (player->hands.size() > 4) {
				playersP4.SetText(("P4:" + to_string(player->hands[4]->points)).c_str());
			}
			if (player->hands.size() > 5) {
				playersP5.SetText(("P5:" + to_string(player->hands[5]->points)).c_str());
			}
			if (player->hands.size() > 6) {
				playersP6.SetText(("P6:" + to_string(player->hands[6]->points)).c_str());
			}
			if (player->hands.size() > 7) {
				playersP7.SetText(("P7:" + to_string(player->hands[7]->points)).c_str());
			}
			if (player->hands.size() > 8) {
				playersP8.SetText(("P8:" + to_string(player->hands[8]->points)).c_str());
			}
			playersTitle.SetText(("Players:  Turn:P" + to_string(player->hands[player->PlayerID]->currentTurn)).c_str());
			playersTitle.Clear();
			playersP0.Clear();
			playersP1.Clear();
			playersP2.Clear();
			playersP3.Clear();
			playersP4.Clear();
			playersP5.Clear();
			playersP6.Clear();
			playersP7.Clear();
			playersP8.Clear();
			//for (size_t i = 0; i < Card::CATEGORY_COUNT; i++)
			//{
			//	cout << Card::ToString(static_cast<Card::CATEGORY>(i)) << ": " << categories[static_cast<Card::CATEGORY>(i)] << ", ";
			//}
			categoriesTitle.Clear();
			categories.Clear();
			ConsoleSetColor(categories.AREA_COLOR_CHAR, categories.AREA_COLOR_BACK);
			ConsoleXY(categories.AREA_START_X, categories.AREA_START_Y);
			cout << Card::ToString(Card::ARABE) << ":    " << to_string(player->hands[player->PlayerID]->categories[Card::ARABE]);
			ConsoleXY(categories.AREA_START_X + 20, categories.AREA_START_Y);
			cout << Card::ToString(Card::BANTU) << ":    " << to_string(player->hands[player->PlayerID]->categories[Card::BANTU]);
			ConsoleXY(categories.AREA_START_X + 40, categories.AREA_START_Y);
			cout << Card::ToString(Card::CHINA) << ":    " << to_string(player->hands[player->PlayerID]->categories[Card::CHINA]);

			ConsoleXY(categories.AREA_START_X, categories.AREA_START_Y+1);
			cout << Card::ToString(Card::ESQUIMAL) << ": " << to_string(player->hands[player->PlayerID]->categories[Card::ESQUIMAL]);
			ConsoleXY(categories.AREA_START_X + 20, categories.AREA_START_Y+1);
			cout << Card::ToString(Card::INDIA) << ":    " << to_string(player->hands[player->PlayerID]->categories[Card::INDIA]);

			ConsoleXY(categories.AREA_START_X, categories.AREA_START_Y+2);
			cout << Card::ToString(Card::MEXICANA) << ": " << to_string(player->hands[player->PlayerID]->categories[Card::MEXICANA]);
			ConsoleXY(categories.AREA_START_X + 20, categories.AREA_START_Y+2);
			cout << Card::ToString(Card::TIROLESA) << ": " << to_string(player->hands[player->PlayerID]->categories[Card::TIROLESA]);

			break;
		default:
			break;
		}
	}
	void UpdateServer(BSS* server) {
		stringstream ss;
		statusType.SetText(PROGRAMTYPE_STRINGS[programtype].c_str());
		statusType.Clear();
		statusConnection.SetText(SERVER_STATES_STRINGS[server->currentState].c_str());
		statusConnection.Clear();
		switch (server->currentState)
		{
		case SERVER_STATES::SERVER_STATE_CONNECTING:
			break;
		case SERVER_STATES::SERVER_STATE_MANAGING:
			playersTitle.SetText("Players");
			playersP0.SetText("Connected:");
			playersP1.SetText("Browsing:");
			playersP3.SetText(to_string(server->players.size()).c_str());
			playersP4.SetText(to_string(server->waitingplayers.size()).c_str());
			playersTitle.Clear();
			playersP0.Clear();
			playersP1.Clear();
			playersP2.Clear();
			playersP3.Clear();
			playersP4.Clear();
			playersP5.Clear();
			playersP6.Clear();
			playersP7.Clear();
			playersP8.Clear();
			PrintGamelist(server->ParseGames());
			break;
		default:
			break;
		}
	}
	void PrintScreen() {
		ConsoleClear();

		statusTitle.Clear();
		statusType.Clear();
		statusConnection.Clear();
		statusGame.Clear();

		gamelistTitle.Clear();
		gamelist.Clear();

		messagesTitle.Clear();
		PrintMessages();

		commandArea.Clear();
		commandArea.SetText(">", false);

		ResetCursor();
	}

	void PrintGamelist(const vector<GameSessionSend> games) {
		gamelist.Clear();
		gamelistTitle.Clear();
		gamelistTitle.SetText("Current games:");
		ConsoleSetColor(gamelist.AREA_COLOR_CHAR, gamelist.AREA_COLOR_BACK);
		for (size_t i = 0; i < games.size(); i++)
		{
			if (i > gamelist.AREA_HEIGHT)
				break;
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

	void PrintHand(const map<Card, bool> cards) {
		gamelist.Clear();
		ConsoleSetColor(gamelist.AREA_COLOR_CHAR, gamelist.AREA_COLOR_BACK);
		int cardCount = 0;
		for (map<Card, bool>::const_iterator it = cards.begin(); it != cards.end(); it++)
		{
			if (it->second)
				cardCount++;
		}
		int i = 0;
		for (map<Card, bool>::const_iterator it = cards.begin(); it != cards.end(); it++)
		{
			ConsoleXY(gamelist.AREA_START_X, gamelist.AREA_START_Y + i);
			if (i > gamelist.AREA_HEIGHT - 1) {
				cout << " ... (" << cardCount - i + 1 << ")";
				break;
			}
			if (it->second) {
				cout << Card::ToString(it->first);
				i++;
			}
		}
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
				if (tempchar != '\n') {
					stringstream << newMessage.at(i);
					j++;
				}
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
	vector<tuple<ConsoleColor, string>> consoleMessages;
};
