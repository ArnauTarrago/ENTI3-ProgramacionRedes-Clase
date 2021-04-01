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
struct Player {
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
        while (true) {
            cout << "Enter server ip" << endl;
            cin >> ip;
            cout << "Enter server port" << endl;
            cin >> port;
            status = socket->connect(ip, port/*, sf::seconds(15.f)*/);
            if (status != sf::Socket::Done) {
                cout << "Connection not available" << endl;
                socket->disconnect();
                cout << "Retry? (y/n)" << endl;
                char retry;
                cin >> retry;
                if (retry == 'Y' || retry == 'y')
                    continue;
                else
                    return;
            }
            else {
                cout << "Connection established with server" << endl;
                if (!Browse(socket))
                    return;
                if (!Load(socket))
                    return;
                if (!Setup())
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
        if (!ReceiveGames(&message, &games)) {
            return false;
        }
		for (size_t i = 0; i < games.size(); i++)
		{
			string auxHasPassword = "ERROR";
			if (games[i].HASPASSWORD == 1) auxHasPassword = "Yes";
			else auxHasPassword = "No";

			cout << "Name: '" << games[i].NAME << "' | Password protected: '" << auxHasPassword << "' | Players: " << games[i].CURRENT_PLAYERS << "/" << games[i].MAX_PLAYERS << endl;
		}
        
        bool waiting = true;
        while (waiting) {
			
			cout << "Do you wish to create 'c' a game or join 'j' an existing game? " << endl;
			cin >> userType;

			switch (userType)
			{
			case 'c':
				char userResponse;
				isCreatingServer = true;

				cout << "Introduce the name of the server:" << endl;
				cin >> serverName;
				cout << "Is the server password protected? (y/n)" << endl;
				cin >> userResponse;

				if (userResponse == 'y')
				{
					cout << "Introduce the password of the server:" << endl;
					cin >> password;
				}

				cout << "Introduce the max number of players:" << endl;
				cin >> maxNumPlayers;

				if (message.send_gameQuery(isCreatingServer, serverName, password, maxNumPlayers))
				{
					if (message.receive_message() >> msg && msg == COMUNICATION_MSGS::MSG_OK)
					{
						waiting = false;
					}
					else
					{
						if (!ReceiveGames(&message, &games)) return false;
					}
				}
				else return false;

				break;
			case 'j':
				char userResponse2;

				isCreatingServer = false;
				cout << "Introduce the name of the server:" << endl;
				cin >> serverName;

				cout << "Is the server password protected? (y/n)" << endl;
				cin >> userResponse2;

				if (userResponse2 == 'y')
				{
					cout << "Introduce the password of the server:" << endl;
					cin >> password;
				}
				
				if (message.send_gameQuery(isCreatingServer, serverName, password, maxNumPlayers))
				{
					if (message.receive_message() >> msg && msg == COMUNICATION_MSGS::MSG_OK)
					{
						waiting = false;
					}
					else
					{
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
                    cout << "Client lost: " << &socket << std::endl;
                    cout << "   Ip : " << socket->getRemoteAddress() << endl;
                    cout << "   Port : " << socket->getRemotePort() << endl;
                    return false;
                }
                else {
                    cout << "Client arrived: " << &socket << std::endl;
                    cout << "   Ip : " << socket->getRemoteAddress() << endl;
                    cout << "   Port : " << socket->getRemotePort() << endl;
                    messages[socket] = new MessageManager(socket);
                    selector.add(*socket);
                }
            }
        }
        if (messages.size() < MAX_PLAYERS) {
            status = listener.listen(localport);
            if (status != Socket::Done) {
                cout << "Could not open ports to listen for clients" << endl;
                return false;
            }
            else {
                cout << "Port opened, listening for clients" << endl;
                while (messages.size() < MAX_PLAYERS - 1) {
                    sf::TcpSocket* socket = new sf::TcpSocket;
                    if (listener.accept(*socket) == sf::Socket::Done)
                    {
                        cout << "Client arrived: " << &socket << std::endl;
                        cout << "   Ip : " << socket->getRemoteAddress() << endl;
                        cout << "   Port : " << socket->getRemotePort() << endl;
                        messages[socket] = new MessageManager(socket);
                        selector.add(*socket);
                    }
                    else
                    {
                        cout << "Client lost: " << &socket << std::endl;
                        cout << "   Ip : " << socket->getRemoteAddress() << endl;
                        cout << "   Port : " << socket->getRemotePort() << endl;
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
                cout << "Client lost: " << &it->first << std::endl;
                cout << "   Ip : " << it->first->getRemoteAddress() << endl;
                cout << "   Port : " << it->first->getRemotePort() << endl;
                return false;
            }
        }
        cout << "You are the player: " << PlayerID << endl;
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
                            cout << "Client lost: " << &it->first << std::endl;
                            cout << "   Ip : " << it->first->getRemoteAddress() << endl;
                            cout << "   Port : " << it->first->getRemotePort() << endl;
                            return false;
                        }
                        players[id] = &client;
                        cout << "Player " << id << " waves at you" << endl;
                    }
                }
            }
        }
        if (PlayerID > 0) {
			seed = players[0]->getRemotePort();
        }
        cout << "Seed: " << seed << endl;

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
