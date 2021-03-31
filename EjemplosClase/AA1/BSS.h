#ifndef BSS_INCLUDED
#define BSS_INCLUDED
#include <SFML/Network.hpp>
#include <chrono>

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
        //if (!message->send_peers(&players)) {
        //    cout << "Connection lost: " << message->peer.socket << endl;
        //    cout << "   Ip : " << message->peer.socket->getRemoteAddress() << endl;
        //    cout << "   Port : " << message->peer.socket->getRemotePort() << endl;
        //}
        //else {
        //}
        players.push_back(message);
        return players.size() >= MAX_PLAYERS;
    }

    void Close() {
        list<MessageManager*> tempplayers = list<MessageManager*>();
        for (list<MessageManager*>::const_iterator it = players.begin(); it != players.end(); it++) {
            if (!(*it)->send_peers(&tempplayers)) {
                cout << "Connection lost: " << (*it)->peer.socket << endl;
                cout << "   Ip : " << (*it)->peer.socket->getRemoteAddress() << endl;
                cout << "   Port : " << (*it)->peer.socket->getRemotePort() << endl;
            }
            else {
                tempplayers.push_back(*it);
                (*it)->peer.socket->disconnect();
            }
        }
        players.clear();
    }
};
struct BSS {
    const int MAX_PLAYERS_MIN = 3;
	vector<Peer> peerList = vector<Peer>();
	list<MessageManager*> players = list<MessageManager*>();
	list<MessageManager*> waitingplayers = list<MessageManager*>();
    list<GameSession*> games = list<GameSession*>();
	int port = 0;
	Socket::Status status = Socket::Status::Disconnected;
	TcpListener dispatcher;
    SocketSelector selector;
    const std::chrono::milliseconds COUNTDOWN_MILLISECONDS = chrono::milliseconds(500);
    chrono::steady_clock::time_point coundown = chrono::steady_clock::now();
	BSS() : MAX_PLAYERS_MIN(MAXPLAYERS) {
        
        while (true) {
            cout << "Enter server port" << endl;
            cin >> port;
            status = dispatcher.listen(port);
            if (status != Socket::Done) {
                cout << "Port not available" << endl;
                dispatcher.close();
                cout << "Retry? (y/n)" << endl;
                char retry;
                cin >> retry;
                if (retry == 'Y' || retry == 'y')
                    continue;
                else
                    return;
            }
            else {
                cout << "Port connected, waiting for clients..." << endl << endl;

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

        selector.add(dispatcher);

        while (true) {
            if (selector.wait()) {
                if (selector.isReady(dispatcher)) {
                    TcpSocket* temp = new TcpSocket();
                    if (dispatcher.accept(*temp) != Socket::Done)
                    {
                        cout << "Connection not accepted" << endl;
                    }
                    else {
                        cout << "Connection: " << &temp << endl;
                        cout << "   Ip : " << temp->getRemoteAddress() << endl;
                        cout << "   Port : " << temp->getRemotePort() << endl;
                        MessageManager* tempMessage = new MessageManager(temp);
                        players.push_back(tempMessage);
                        waitingplayers.push_back(tempMessage);
                        selector.add(*tempMessage->peer.socket);
                        if (!SendGames(tempMessage)) {
                            cout << "Connection lost: " << &temp << endl;
                            cout << "   Ip : " << temp->getRemoteAddress() << endl;
                            cout << "   Port : " << temp->getRemotePort() << endl;
                        }
                    }
                }
                else {

                    for (list<MessageManager*>::iterator it = players.begin(); it != players.end(); it++)
                    {
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
                                                games.remove((*it2));
                                                (*it)->send_ok();
                                                waitingplayers.remove((*it));
                                                notjoined = false;
                                                (*it2)->Close();
                                                for (list<MessageManager*>::iterator it3 = (*it2)->players.begin(); it3 != (*it2)->players.end(); it3++) {
                                                    players.remove((*it3));
                                                    delete (*it3)->peer.socket;
                                                    delete* it3;
                                                }
                                                delete* it2;
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
                                cout << "Connection lost: " << &client << endl;
                                cout << "   Ip : " << client.getRemoteAddress() << endl;
                                cout << "   Port : " << client.getRemotePort() << endl;
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
                                players.remove((*it));
                                delete (*it)->peer.socket;
                                delete* it;
                            }
                        }
                    }
                }
            }
            if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - coundown) > COUNTDOWN_MILLISECONDS) {
                coundown = chrono::steady_clock::now();
                //SEND GAMELIST
            }
        }
        
	}

    bool SendGames(MessageManager* messages) {
        vector<GameSessionSend> tempgames;
        tempgames.reserve(games.size());
        for (list<GameSession*>::iterator it = games.begin(); it != games.end(); it++) {
            tempgames.push_back(make_tuple((*it)->NAME, (*it)->players.size(), (*it)->MAX_PLAYERS, !(*it)->PASSWORD.empty()));
        }
        return messages->send_gamelist(&tempgames);
    }
};

#endif // BSS_INCLUDED
