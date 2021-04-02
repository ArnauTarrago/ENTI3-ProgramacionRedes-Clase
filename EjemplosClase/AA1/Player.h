#ifndef PLAYER_INCLUDED
#define PLAYER_INCLUDED
#include "Card.h"

/*
c
localhost
50000
c
asd
123
3

*/

/*
c
localhost
50000
j
asd
123

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
                if (retry == 'Y' || retry == 'y')
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
        if (!ReceiveGames(&message, &games)) {
            return false;
        }
        
        bool waiting = true;
        while (waiting) {

            AddMessage("Do you wish to create 'c' a game or join 'j' an existing game?");
            userType = GetInput_Char();
			switch (userType)
			{
			case 'c':
				char userResponse;
				isCreatingServer = true;

				AddMessage("Introduce the name of the server:");
				serverName = GetInput_String();
				AddMessage("Is the server password protected? (y/n)");
				userResponse = GetInput_Char();

				if (userResponse == 'y' || userResponse == 'Y')
				{
					AddMessage("Introduce the password of the server:");
                    password = GetInput_String();
				}

				AddMessage("Introduce the max number of players:");
				maxNumPlayers = GetInput_Int();

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
						if (!ReceiveGames(&message, &games)) return false;
					}
				}
				else return false;

				break;
			case 'j':

				isCreatingServer = false;
				AddMessage("Introduce the name of the server:");
                serverName = GetInput_String();

				AddMessage("Is the server password protected? (y/n)");
                userResponse = GetInput_Char();

				if (userResponse == 'y' || userResponse == 'Y')
				{
					AddMessage("Introduce the password of the server:");
                    password = GetInput_String();
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
						if (!ReceiveGames(&message, &games)) return false;							
					}
				}
				else return false;

				break;
			default:
				break;
			}
        }
        return true;
    }

    bool ReceiveGames(MessageManager* messages, vector<GameSessionClient>* games) {
        vector<GameSessionSend> _games;
        if (!messages->receive_gamelist(&_games)) {
            return false;
        }
        PrintGamelist(_games);
        games->reserve(_games.size());
        for (size_t i = 0; i < _games.size(); i++)
        {
            games->push_back(GameSessionClient(get<0>(_games.at(i)), get<1>(_games.at(i)), get<2>(_games.at(i)), get<3>(_games.at(i))));
        }
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
        }
        for (size_t i = 0; i < deck.deck.size(); i++)
        {
            hands[i % MAX_PLAYERS]->add(*deck.deck[i]);
        }
        hands[PlayerID]->Print();

        return true;
    }

	void Play() {

	}
};

#endif
