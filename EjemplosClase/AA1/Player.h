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
    map<TcpSocket*, MessageManager*> messages;
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
                Load(socket);
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
                    if (selector.isReady(listener))
                    {
                        // The listener is ready: there is a pending connection
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
                }
                listener.close();
                selector.remove(listener);
                return true;
            }
        }
        else {
            return true;
        }
    }

    void Setup() {
        if (PlayerID <= 0) {
            srand(time(NULL));
            seed = rand();
            srand(seed);
        }
        else {

        }
    }

	void Play() {

	}
};

#endif
