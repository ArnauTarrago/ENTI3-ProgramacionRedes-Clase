#ifndef BSS_INCLUDED
#define BSS_INCLUDED
#include <SFML/Network.hpp>

using namespace std;
using namespace sf;
struct GameSession {
    const string NAME;
    const string PASSWORD;
    const int MAX_PLAYERS;

    list<MessageManager*> players = list<MessageManager*>();

    GameSession(string _NAME, string _PASSWORD, int _MAX_PLAYERS) : NAME(_NAME), PASSWORD(_PASSWORD), MAX_PLAYERS(_MAX_PLAYERS) {};
    bool inline Join(string _NAME, string _PASSWORD) {
        return _NAME == NAME && _PASSWORD == PASSWORD;
    }

    bool Add(MessageManager* message) {
        players.push_back(message);
        return players.size() >= MAX_PLAYERS;
    }

    void Close() {
        list<MessageManager*> tempplayers = list<MessageManager*>();
        for (list<MessageManager*>::const_iterator it = players.begin(); it != players.end(); it++) {
            if (!(*it)->send_peers(&tempplayers)) {
                AddConnection((*it)->peer.socket, false);
            }
            else {
                tempplayers.push_back(*it);
                (*it)->peer.socket->disconnect();
            }
        }
        //players.clear();
    }
};
enum SERVER_STATES {
    SERVER_STATE_CONNECTING, SERVER_STATE_MANAGING
    , STATE_COUNT
};
const static string SERVER_STATES_STRINGS[] = {
    "CONNECTING",
    "MANAGING",
    "2",
};
struct BSS {
    SERVER_STATES currentState = SERVER_STATES::SERVER_STATE_CONNECTING;
    const int MAX_PLAYERS_MIN = 3;
    const int MAX_PLAYERS_MAX = 9;
	vector<Peer> peerList = vector<Peer>();
	list<MessageManager*> players = list<MessageManager*>();
	list<MessageManager*> waitingplayers = list<MessageManager*>();
    list<GameSession*> games = list<GameSession*>();
	int port = 0;
	Socket::Status status = Socket::Status::Disconnected;
	TcpListener dispatcher;
    SocketSelector selector;
	BSS() : MAX_PLAYERS_MIN(MAXPLAYERS) {
        UpdateServer(this);
        while (true) {
            AddMessage("Enter server port");
            port = GetInput_Int();
            status = dispatcher.listen(port);
            if (status != Socket::Done) {
                AddMessage("Port not available. Retry? (y/n)", RED);
                dispatcher.close();
                char retry;
                retry = GetInput_Char();
                if (retry == 'Y' || retry == 'y')
                    continue;
                else
                    return;
            }
            else {
                AddMessage("Port connected, waiting for clients...\n");

                break;
            }
        }
        //{
        //    GameSession* tempGame = new GameSession("TEST1", "", 10);
        //    games.push_back(tempGame);
        //}
        //{
        //    GameSession* tempGame = new GameSession("TEST2", "123", 10);
        //    games.push_back(tempGame);
        //}

        currentState = SERVER_STATES::SERVER_STATE_MANAGING;
        UpdateServer(this);
        selector.add(dispatcher);

        while (true) {
            if (selector.wait()) {
                if (selector.isReady(dispatcher)) {
                    TcpSocket* temp = new TcpSocket();
                    if (dispatcher.accept(*temp) != Socket::Done)
                    {
                        AddMessage("Connection not accepted", RED);
                    }
                    else {
                        AddConnection(temp, true);
                        MessageManager* tempMessage = new MessageManager(temp);
                        players.push_back(tempMessage);
                        waitingplayers.push_back(tempMessage);
                        selector.add(*tempMessage->peer.socket);
                        if (!SendGames(tempMessage)) {
                            AddConnection(temp, false);
                        }
                    }
                    UpdateServer(this);
                }
                else {
                    for (list<MessageManager*>::iterator it = players.begin(); it != players.end(); it++)
                    {
                        if (players.size() <= 0)
                        {
                            break;
                        }
                        sf::TcpSocket& client = *(*it)->peer.socket;
                        if (selector.isReady(client)) {
                            bool create = false;
                            string name;
                            string password;
                            int maxplayers;
                            if ((*it)->receive_gameQuery(&create, &name, &password, &maxplayers)) {
                                if (create) {
                                    bool notcreated = true;
                                    for (list<GameSession*>::iterator it2 = games.begin(); it2 != games.end(); it2++) {
                                        if ((*it2)->NAME == name) {
                                            (*it)->send_ko();
                                            if (!SendGames((*it))) {

                                            }
                                            notcreated = false;
                                            break;
                                        }
                                    }
                                    if (notcreated) {
                                        maxplayers < MAX_PLAYERS_MIN ? maxplayers = MAX_PLAYERS_MIN : maxplayers = maxplayers;
                                        maxplayers > MAX_PLAYERS_MAX ? maxplayers = MAX_PLAYERS_MAX : maxplayers = maxplayers;
                                        GameSession* tempGame = new GameSession(name, password, maxplayers);
                                        tempGame->players.push_back((*it));
                                        games.push_back(tempGame);
                                        waitingplayers.remove((*it));
                                        (*it)->send_ok();
                                    }
                                }
                                else {
                                    bool notjoined = true;
                                    for (list<GameSession*>::iterator it2 = games.begin(); it2 != games.end(); it2++) {
                                        if ((*it2)->Join(name, password)) {
                                            if ((*it2)->Add((*it))) {
                                                (*it)->send_ok();
                                                waitingplayers.remove((*it));
                                                notjoined = false;
                                                (*it2)->Close();
                                                for (list<MessageManager*>::iterator it3 = (*it2)->players.begin(); it3 != (*it2)->players.end(); it3++) {
                                                    players.remove((*it3));
                                                    delete (*it3)->peer.socket;
                                                    delete* it3;
                                                }
                                                ((*it2))->players.clear();
                                                GameSession* temp = *it2;
                                                games.remove((*it2));
                                                delete temp;
                                            }
                                            else {
                                                (*it)->send_ok();
                                                waitingplayers.remove((*it));
                                                notjoined = false;
                                            }
                                            break;
                                        }
                                    }
                                    if (notjoined) {
                                        (*it)->send_ko();
                                        if (!SendGames((*it))) {

                                        }
                                    }
                                }
                            }
                            else {
                                AddConnection(&client, false);
                                for (list<GameSession*>::iterator it2 = games.begin(); it2 != games.end(); it2++) {
                                    try {
                                        (*it2)->players.remove((*it));
                                    }
                                    catch (const exception& ex) {
                                    }
                                }
                                try {
                                    waitingplayers.remove((*it));
                                }
                                catch (const exception& ex) {
                                }
                                MessageManager* temp = *it;
                                players.remove((*it));
                                delete (temp)->peer.socket;
                                delete temp;
                            }
                            UpdateServer(this);
                            break;
                        }
                    }
                }
            }
        }
        
	}
    const vector<GameSessionSend> ParseGames() {
        vector<GameSessionSend> tempgames;
        tempgames.reserve(games.size());
        for (list<GameSession*>::const_iterator it = games.begin(); it != games.end(); it++) {
            tempgames.push_back(make_tuple((*it)->NAME, (*it)->players.size(), (*it)->MAX_PLAYERS, !(*it)->PASSWORD.empty()));
        }
        return tempgames;
    }
    bool SendGames(MessageManager* messages) {
        vector<GameSessionSend> tempgames = ParseGames();
        PrintGamelist(tempgames);
        return messages->send_gamelist(&tempgames);
    }
};

#endif // BSS_INCLUDED
