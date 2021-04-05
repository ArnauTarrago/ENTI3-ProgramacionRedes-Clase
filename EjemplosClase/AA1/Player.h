#ifndef PLAYER_INCLUDED
#define PLAYER_INCLUDED
#include "Card.h"

/*
c
localhost
50000
c
asd
n
3

*/

/*
c
localhost
50000
j
asd
n

*/

using namespace std;
using namespace sf;
struct GameSessionClient {
    const string NAME;
    const bool HASPASSWORD;
    const int CURRENT_PLAYERS;
    const int MAX_PLAYERS;

    GameSessionClient(string _NAME, int _CURRENT_PLAYERS, int _MAX_PLAYERS, bool _HASPASSWORD) : NAME(_NAME), CURRENT_PLAYERS(_CURRENT_PLAYERS), MAX_PLAYERS(_MAX_PLAYERS), HASPASSWORD(_HASPASSWORD) {};
};
enum PLAYER_STATES {
    PLAYER_STATE_CONNECTING, PLAYER_STATE_BROWSING, PLAYER_STATE_LOADING, PLAYER_STATE_SETUP, PLAYER_STATE_INGAME
    , PLAYER_STATE_COUNT
};
const static string PLAYER_STATES_STRINGS[] = {
    "CONNECTING",
    "BROWSING",
    "LOADING",
    "SETUP",
    "INGAME",
    "5",
};
enum PLAYER_STATES_INGAME {
    PLAYER_STATE_INGAME_NOTINGAME
    , PLAYER_STATE_INGAME_COUNT
};
const static string PLAYER_STATES_INGAME_STRINGS[] = {
    "NOTINGAME",
    "1",
};
struct Player {
    PLAYER_STATES currentState = PLAYER_STATES::PLAYER_STATE_CONNECTING;
    PLAYER_STATES_INGAME currentStateIngame = PLAYER_STATES_INGAME::PLAYER_STATE_INGAME_NOTINGAME;
    const int MAX_PLAYERS;
    int port;
    int PlayerID;
    int seed;
    IpAddress ip;
    TcpListener dispatcher;
    SocketSelector selector;
    Deck deck;
    map<int, TcpSocket*> players;
    map<TcpSocket*, MessageManager*> messages;
    map<int, Hand*> hands;
	Player() : MAX_PLAYERS(MAXPLAYERS) {
        TcpSocket * socket = new TcpSocket();
        Socket::Status status;
        UpdateClient(this);
        while (true) {
            AddMessage("Enter server ip:");
            ip = GetInput_String();
            AddMessage("Enter server port:");
            port = GetInput_Int();
            status = socket->connect(ip, port/*, sf::seconds(15.f)*/);
            if (status != sf::Socket::Done) {
                AddMessage("Connection not available. Retry? (y/n)", RED);
                socket->disconnect();
                char retry = GetInput_Char();
                if (GetInput_Confirmation())
                    continue;
                else
                    return;
            }
            else {
                AddMessage("Connection established with server");
                currentState = PLAYER_STATES::PLAYER_STATE_BROWSING;
                UpdateClient(this);
                if (!Browse(socket))
                    return;
                currentState = PLAYER_STATES::PLAYER_STATE_LOADING;
                UpdateClient(this);
                if (!Load(socket))
                    return;
                currentState = PLAYER_STATES::PLAYER_STATE_SETUP;
                UpdateClient(this);
                if (!Setup())
                    return;
                currentState = PLAYER_STATES::PLAYER_STATE_INGAME;
                UpdateClient(this);
				if (!Play())
					return;
                break;
            }
        }
	}

    bool Browse(TcpSocket* _socket) {
		bool isCreatingServer;
		char userType;
		string serverName = "";
		string password = "";
		int maxNumPlayers = -1;
		Packet pack;
		int msg = COMUNICATION_MSGS::MSG_NULL;

        MessageManager message = MessageManager(_socket);
        vector<GameSessionClient> games;
        GameSessionFilter* filter = new GameSessionFilter();
        GameSessionOrder* order = new GameSessionOrder();
        if (!ReceiveGames(&message, &games)) {
            return false;
        }
        
        bool waiting = true;
        while (waiting) {

            AddMessage("Do you wish to create 'c' a game or join 'j' an existing game, or filter/sort 'f' the list?");
            userType = GetInput_Char();
            PrintGamelist(ParseBackGames(&games), filter, order);
			switch (userType)
			{
			case 'c':
				isCreatingServer = true;

				AddMessage("Introduce the name of the server:");
				serverName = GetInput_String();
                PrintGamelist(ParseBackGames(&games), filter, order);
				AddMessage("Is the server password protected? (y/n)");

				if (GetInput_Confirmation())
				{
                    PrintGamelist(ParseBackGames(&games), filter, order);
					AddMessage("Introduce the password of the server:");
                    password = GetInput_String();
                    PrintGamelist(ParseBackGames(&games), filter, order);
				}

				AddMessage("Introduce the max number of players:");
				maxNumPlayers = GetInput_Int();
                PrintGamelist(ParseBackGames(&games), filter, order);

				if (message.send_gameQuery(isCreatingServer, serverName, password, maxNumPlayers))
				{
					if (message.receive_message() >> msg && msg == COMUNICATION_MSGS::MSG_OK)
					{
                        AddMessage("Game created successfully", GREEN);
						waiting = false;
					}
					else
					{
                        AddMessage("Game not created", RED);
						if (!ReceiveGames(&message, &games, filter, order)) return false;
					}
				}
				else return false;

				break;
			case 'j':

				isCreatingServer = false;
				AddMessage("Introduce the name of the server:");
                serverName = GetInput_String();
                PrintGamelist(ParseBackGames(&games), filter, order);

				AddMessage("Is the server password protected? (y/n)");

				if (GetInput_Confirmation())
				{
                    PrintGamelist(ParseBackGames(&games), filter, order);
					AddMessage("Introduce the password of the server:");
                    password = GetInput_String();
                    PrintGamelist(ParseBackGames(&games), filter, order);
				}
				
				if (message.send_gameQuery(isCreatingServer, serverName, password, maxNumPlayers))
				{
					if (message.receive_message() >> msg && msg == COMUNICATION_MSGS::MSG_OK)
					{
                        AddMessage("Game joined successfully", GREEN);
						waiting = false;
					}
					else
					{
                        AddMessage("Game not joined", RED);
						if (!ReceiveGames(&message, &games, filter, order)) return false;
					}
				}
				else return false;

				break;
			default:
                FilterGamelist(ParseBackGames(&games), filter, order);
				break;
			}
        }
        return true;
    }

    vector<GameSessionSend> ParseBackGames(const vector<GameSessionClient>* games) {
        vector<GameSessionSend> tempgames;
        tempgames.reserve(games->size());
        for (vector<GameSessionClient>::const_iterator it = games->begin(); it != games->end(); it++) {
            tempgames.push_back(make_tuple(it->NAME, it->CURRENT_PLAYERS, it->MAX_PLAYERS, it->HASPASSWORD));
        }
        return tempgames;
    }

    bool ReceiveGames(MessageManager* messages, vector<GameSessionClient>* games, GameSessionFilter* filter = nullptr, GameSessionOrder* order = nullptr) {
        vector<GameSessionSend> _games;
        if (!messages->receive_gamelist(&_games)) {
            return false;
        }
        games->reserve(_games.size());
        for (size_t i = 0; i < _games.size(); i++)
        {
            games->push_back(GameSessionClient(get<0>(_games.at(i)), get<1>(_games.at(i)), get<2>(_games.at(i)), get<3>(_games.at(i))));
        }
        PrintGamelist(_games, filter, order);
        return true;
    }

    bool Load(TcpSocket* _socket) {
        MessageManager message = MessageManager(_socket);
        vector<Peer> peerList = vector<Peer>();
        if (!message.receive_peers(&peerList))
            return false;
        int localport = _socket->getLocalPort();
		seed = localport;
        _socket->disconnect();
        TcpListener listener;
        Socket::Status status;
        PlayerID = peerList.size();
        if (PlayerID > 0) {
            for (size_t i = 0; i < peerList.size(); i++)
            {
                TcpSocket* socket = new TcpSocket();
                status = socket->connect(peerList.at(i).ip, peerList.at(i).port);
                if (status != sf::Socket::Done) {
                    AddConnection(socket, false);
                    return false;
                }
                else {
                    AddConnection(socket, true);
                    messages[socket] = new MessageManager(socket);
                    selector.add(*socket);
                }
            }
        }
        if (messages.size() < MAX_PLAYERS) {
            status = listener.listen(localport);
            if (status != Socket::Done) {
                AddMessage("Could not open ports to listen for clients");
                return false;
            }
            else {
                AddMessage("Port opened, listening for clients");
                while (messages.size() < MAX_PLAYERS - 1) {
                    sf::TcpSocket* socket = new sf::TcpSocket;
                    if (listener.accept(*socket) == sf::Socket::Done)
                    {
                        AddConnection(socket, true);
                        messages[socket] = new MessageManager(socket);
                        selector.add(*socket);
                    }
                    else
                    {
                        AddConnection(socket, false);
                        return false;
                    }
                }
                listener.close();
                return true;
            }
        }
        else {
            return true;
        }
    }

    bool Setup() {
        for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
        {
            if (!it->second->send_greet(PlayerID)) {
                AddConnection(it->first, false);
                return false;
            }
        }
        ostringstream stringstream;
        stringstream << "You are the player: " << PlayerID;
        AddMessage(stringstream.str());
        players[PlayerID] = nullptr;
        while (players.size() < MAX_PLAYERS) {
            if (selector.wait())
            {
                for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
                {
                    int id = -1;
                    sf::TcpSocket& client = *it->first;
                    if (selector.isReady(client)) {
                        if (!messages[&client]->receive_greet(&id)) {
                            AddConnection(it->first, false);
                            return false;
                        }
                        players[id] = &client;
                        ostringstream stringstream;
                        stringstream << "Player " << id << " waves at you";
                        AddMessage(stringstream.str());
                    }
                }
            }
        }
        if (PlayerID > 0) {
			seed = players[0]->getRemotePort();
        }
        stringstream.str("");
        stringstream << "Seed: " << seed;
        AddMessage(stringstream.str());

        deck = Deck();
        deck.Shuffle(seed);

        for (size_t i = 0; i < MAX_PLAYERS; i++)
        {
            hands[i] = new Hand();
			hands[i]->isActive = true;
			hands[i]->currentTurn = 0;
        }
        for (size_t i = 0; i < deck.deck.size(); i++)
        {
            hands[i % MAX_PLAYERS]->add(*deck.deck[i]);
        }
        //hands[PlayerID]->Print();

        return true;
    }

	bool Play() {
		string auxString = "";
		int playerSelected, categorySelected, numberSelected;
		ostringstream stringstream;
		
		while (true) 
		{
			if (CheckIfGameOverConditionsApply())
			{
				DoGameOver();
				break;
			}

			if (hands[PlayerID]->currentTurn == 0)
			{
				// CHEATING CONTROL
			}			

			while (hands[PlayerID]->currentTurn == PlayerID)
			{
				AddMessage("Which player would you like to ask a card from? Type the number:");
				for (size_t i = 0; i < players.size(); i++)
				{
					if(i != PlayerID) stringstream << " - " << i << " - " << " Player " << i << "\n";		
				}
				AddMessage(stringstream.str());
				stringstream.str("");
				playerSelected = GetInput_Int();
				UpdateClient(this);

				playerSelected >= players.size() ? playerSelected = players.size() - 1 : playerSelected = playerSelected;
				playerSelected < 0 ? playerSelected = 0 : playerSelected = playerSelected;
				if (playerSelected == PlayerID)
				{
					AddMessage("You can't ask yourself!", RED);
					break;
				}

				AddMessage("Which family would you like to choose? Type the number:");
				for (size_t i = 0; i < Card::CATEGORY_COUNT; i++)
				{
					stringstream << " - " << i << " - " << Card::ToString(static_cast<Card::CATEGORY>(i)) << "\n";					
					
				}
				AddMessage(stringstream.str());
				stringstream.str("");
				categorySelected = GetInput_Int();
				UpdateClient(this);
				categorySelected >= Card::CATEGORY_COUNT ? categorySelected = Card::CATEGORY_COUNT - 1 : categorySelected = categorySelected;
				categorySelected < 0 ? categorySelected = 0 : categorySelected = categorySelected;				

				AddMessage("Which person would you like to choose? Type the number:");
				for (size_t i = 0; i < Card::NUMBER_COUNT; i++)
				{
					stringstream << " - " << i << " - " << Card::ToString(static_cast<Card::NUMBER>(i)) << "\n";					

				}
				AddMessage(stringstream.str());
				stringstream.str("");
				numberSelected = GetInput_Int();
				UpdateClient(this);
				numberSelected >= Card::NUMBER_COUNT ? numberSelected = Card::NUMBER_COUNT - 1 : numberSelected = numberSelected;
				numberSelected < 0 ? numberSelected = 0 : numberSelected = numberSelected;
				

				Card auxCard(categorySelected, numberSelected);

				//Player sends the card request to the other peers
				for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
				{
					if (!it->second->send_requestCard(playerSelected, categorySelected, numberSelected)) {
						// TODO: Control
					}
				}

				if (DoesPlayerHaveCard(playerSelected, auxCard))
				{
					PlayerStealsCard(PlayerID, playerSelected, auxCard);
				}
				else
				{
					stringstream << "Player " << playerSelected << " doesn't have the card " << Card::ToString(auxCard.CAT) << " (" << auxCard.CAT << ") " << Card::ToString(auxCard.NUM) << " (" << auxCard.NUM << ").";
					AddMessage(stringstream.str());
					stringstream.str("");
					UpdateClient(this);
					PassTurn();
				}

				if (CheckIfGameOverConditionsApply())
				{
					break;
				}
			}

			if (hands[PlayerID]->currentTurn != PlayerID)
			{
				if (selector.wait())
				{
					for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
					{
						sf::TcpSocket& client = *it->first;
						if (selector.isReady(client)) {
							if (messages[&client]->receive_requestCard(&playerSelected, &categorySelected, &numberSelected))
							{
								Card auxCard(categorySelected, numberSelected);

								if (DoesPlayerHaveCard(playerSelected, auxCard))
								{
									PlayerStealsCard(hands[PlayerID]->currentTurn, playerSelected, auxCard);
								}
								else
								{
									stringstream << "Player " << playerSelected << " doesn't have the card " << Card::ToString(auxCard.CAT) << " (" << auxCard.CAT << ") " << Card::ToString(auxCard.NUM) << " (" << auxCard.NUM << ").";
									AddMessage(stringstream.str());
									stringstream.str("");
									UpdateClient(this);
									PassTurn();
								}

								if (CheckIfGameOverConditionsApply())
								{
									break;
								}
							}
						}
					}
				}
			}
			
		}
		return true;		
	}

	bool DoesPlayerHaveCard(int player, Card card)
	{
		return hands[player]->has(card);
	}

	void PlayerStealsCard(int playerThief, int playerVictim, Card card)
	{
		ostringstream stringstream;

		stringstream << "Player " << playerThief << " has stolen the card " << Card::ToString(card.CAT) << " (" << card.CAT << ") " << Card::ToString(card.NUM) << " (" << card.NUM << ") from Player "  << playerVictim << ".";
		AddMessage(stringstream.str());
		stringstream.str("");

		hands[playerThief]->add(card);
		hands[playerVictim]->remove(card);
		UpdateClient(this);
	}

	void PassTurn()
	{
		ostringstream stringstream;

		for (size_t i = 0; i < players.size(); i++)
		{
			do
			{
				hands[i]->currentTurn++;
				if (hands[i]->currentTurn >= players.size()) hands[i]->currentTurn = 0;
			} while (!hands[hands[i]->currentTurn]->isActive);
		}
		stringstream << "It's the turn of " << hands[PlayerID]->currentTurn;
		AddMessage(stringstream.str(), GREEN);
		stringstream.str("");
		UpdateClient(this);
	}
		
	int GetNumberOfActivePlayers()
	{
		int numberOfActivePlayers = 0;
		for (size_t i = 0; i < players.size(); i++)
		{
			if (hands[i]->isActive) numberOfActivePlayers++;
		}
		return numberOfActivePlayers;
	}

	bool AreThereAnyCardsLeft()
	{
		for (size_t i = 0; i < players.size(); i++)
		{
			if (hands[i]->numberOfCards > 0) return true;
		}
		return false;
	}

	void DoGameOver()
	{
		ostringstream stringstream;

		int auxPoints = 0;
		int playerWinnerID = -1;

		for (size_t i = 0; i < players.size(); i++)
		{
			if (hands[i]->points > auxPoints)
			{
				auxPoints = hands[i]->points;
				playerWinnerID = i;
			}
		}
		if (playerWinnerID == -1) stringstream << "All of you lose! Congratulations!";
		else stringstream << "Game Over! The winner is: 'Player " << playerWinnerID << "'!";
		AddMessage(stringstream.str(), RED);
		stringstream.str("");
	}

	bool CheckIfGameOverConditionsApply()
	{		
		return ( GetNumberOfActivePlayers() <= 2 || !AreThereAnyCardsLeft() );
	}
};

#endif
