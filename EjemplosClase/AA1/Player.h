#ifndef PLAYER_INCLUDED
#define PLAYER_INCLUDED
#include "Card.h"
#include <map>

using namespace std;
using namespace sf;
struct Player {
    const int MAX_PLAYERS;
    int port;
    int PlayerID;
    int seed;
    IpAddress ip;
    TcpListener dispatcher;
    SocketSelector selector;
    map<int, TcpSocket*> players;
    map<TcpSocket*, MessageManager*> messages;
    map<TcpSocket*, Hand*> hands;
	Player() : MAX_PLAYERS(MAXPLAYERS) {
        TcpSocket * socket = new TcpSocket();
        Socket::Status status;
        while (true) {
            cout << "Introduce un puerto" << endl;
            cin >> port;
            cout << "Introduce una ip" << endl;
            cin >> ip;
            status = socket->connect(ip, port/*, sf::seconds(15.f)*/);
            if (status != sf::Socket::Done) {
                cout << "Conexión no aceptada" << endl;
                socket->disconnect();
                cout << "¿Desea reintentar? (y/n)" << endl;
                char retry;
                cin >> retry;
                if (retry == 'Y' || retry == 'y')
                    continue;
                else
                    return;
            }
            else {
                cout << "Conexión establecida" << endl;
                if (!Load(socket))
                    return;
                if (!Setup())
                    return;
                break;
            }
        }
	}

    bool Load(TcpSocket* _socket) {
        vector<Peer> peerList = vector<Peer>();

        MessageManager message = MessageManager(_socket);
        if (!message.receive_peers(&peerList))
            return false;
        int localport = _socket->getLocalPort();
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
                    cout << "Conexión no aceptada" << endl;
                    return false;
                }
                else {
                    cout << "Conexión establecida" << endl;
                    messages[socket] = new MessageManager(socket);
                    selector.add(*socket);
                }
            }
        }
        if (messages.size() < MAX_PLAYERS) {
            status = listener.listen(localport);
            if (status != Socket::Done) {
                cout << "Puerto no vinculado" << endl;
                return false;
            }
            else {
                cout << "Puerto vinculado, esperando clientes" << endl;
                while (messages.size() < MAX_PLAYERS) {
                    sf::TcpSocket* socket = new sf::TcpSocket;
                    if (listener.accept(*socket) == sf::Socket::Done)
                    {
                        std::cout << "Llega el cliente con puerto: " << socket->getRemotePort() << std::endl;
                        messages[socket] = new MessageManager(socket);
                        selector.add(*socket);
                    }
                    else
                    {
                        std::cout << "Error al recoger conexión nueva\n";
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
                cout << "Conexión perdida" << endl;
                return false;
            }
            else {
                cout << "Eres el jugador: " << PlayerID << endl;
            }
        }
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
                            cout << "Conexión perdida" << endl;
                            return false;
                        }
                        players[id] = &client;
                        cout << "El jugador " << id << "te saluda." << endl;
                    }
                }
            }
        }
        if (PlayerID <= 0) {
            srand(time(NULL));
            seed = rand();
            srand(seed);
            for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
            {
                if (!it->second->send_seed(seed)) {
                    cout << "Conexión perdida" << endl;
                    return false;
                }
            }
        }
        else {
            seed = -1;
            while (seed < 0) {

                if (selector.wait())
                {
                    for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
                    {
                        sf::TcpSocket& client = *it->first;
                        if (selector.isReady(client)) {
                            if (!messages[&client]->receive_seed(&seed)) {
                                cout << "Conexión perdida" << endl;
                                return false;
                            }
                        }
                    }
                }
            }
        }
        Deck deck = Deck();
        deck.Shuffle(seed);

        return true;
    }

	void Play() {

	}
};

#endif
