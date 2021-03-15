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

    bool Add(MessageManager*) {

        return players.size() >= MAX_PLAYERS;
    }

    void Close() {

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
                cout << "Port connected, waiting for " << MAX_PLAYERS << " clients..." << endl << endl;

                break;
            }
        }

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
                                    maxplayers < MAX_PLAYERS_MIN ? maxplayers = MAX_PLAYERS_MIN : maxplayers = maxplayers;
                                    GameSession* tempGame = new GameSession(name, password, maxplayers);
                                    tempGame->players.push_back((*it));
                                    games.push_back(tempGame);
                                    waitingplayers.remove((*it));
                                }
                                else {
                                    bool notjoined = true;
                                    for (list<GameSession*>::iterator it2 = games.begin(); it2 != games.end(); it2++) {
                                        if ((*it2)->Join(name, password)) {
                                            if ((*it2)->Add((*it))) {
                                                (*it2)->Close();
                                                games.remove((*it2));
                                            }
                                            (*it)->send_ok();
                                            waitingplayers.remove((*it));
                                            notjoined = false;
                                            break;
                                        }
                                    }
                                    if (notjoined)
                                        (*it)->send_ko();
                                }
                            }
                            else {
                                cout << "Connection lost: " << &client << endl;
                                cout << "   Ip : " << client.getRemoteAddress() << endl;
                                cout << "   Port : " << client.getRemotePort() << endl;
                            }
                        }
                    }
                }
            }
        }

        IpAddress ip = IpAddress::LocalHost;
        cout << ip.getLocalAddress() << ":" << to_string(port) << endl;
        while (peerList.size() < MAX_PLAYERS) {
            TcpSocket* temp = new TcpSocket();
            if (dispatcher.accept(*temp) != Socket::Done)
            {
                cout << "Connection not accepted" << endl;
            }
            else {
                cout << "Connection: " << &temp << " (" << peerList.size()+1 << "/" << MAX_PLAYERS << ")" <<  endl;
                cout << "   Ip : " << temp->getRemoteAddress() << endl;
                cout << "   Port : " << temp->getRemotePort() << endl;
                // Informar al socket de la información de la gente en la peerList
                MessageManager message = MessageManager(temp);
                if (!message.send_peers(&peerList)) {
                    cout << "Connection lost: " << &temp << endl;
                    cout << "   Ip : " << temp->getRemoteAddress() << endl;
                    cout << "   Port : " << temp->getRemotePort() << endl;
                }
                else {
                    peerList.push_back(Peer(temp->getRemoteAddress(), temp->getRemotePort()));
                }
                temp->disconnect();
            }
        }
        dispatcher.close();
        cout << "All players connected. Closing." << endl;
	}
};

#endif // BSS_INCLUDED
