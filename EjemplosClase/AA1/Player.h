#ifndef PLAYER_INCLUDED
#define PLAYER_INCLUDED
#include "Card.h"

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
        if (PlayerID <= 0) {
            srand(time(NULL));
            seed = rand();
			// PARA EVITAR ENVIAR LA SEED A OTROS, SE PUEDE USAR EL PUERTO LOCAL DE PLAYER 0 COMO SEED. SE TENDRIA QUE QUITAR EL ENVIO Y RECIBO DE SEEDS SI SE USA ESTE METODO
			//seed = players[0]->getLocalPort();
            srand(seed);
            for (map<TcpSocket*, MessageManager*>::iterator it = messages.begin(); it != messages.end(); it++)
            {
                if (!it->second->send_seed(seed)) {
                    cout << "Client lost: " << &it->first << std::endl;
                    cout << "   Ip : " << it->first->getRemoteAddress() << endl;
                    cout << "   Port : " << it->first->getRemotePort() << endl;
                    return false;
                }
            }
        }
        else {
            messages[players[0]]->receive_seed(&seed);
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
