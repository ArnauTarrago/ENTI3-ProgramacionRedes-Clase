#ifndef BSS_INCLUDED
#define BSS_INCLUDED
#include <SFML/Network.hpp>
using namespace std;
using namespace sf;
struct BSS {
    const int MAX_PLAYERS = 3;
	vector<Peer> peerList = vector<Peer>();
	int port = 0;
	Socket::Status status = Socket::Status::Disconnected;
	TcpListener dispatcher;
	BSS() : MAX_PLAYERS(MAXPLAYERS) {
        while (true) {
            cout << "Introduce un puerto" << endl;
            cin >> port;
            status = dispatcher.listen(port);
            if (status != Socket::Done) {
                cout << "Puerto no vinculado" << endl;
                dispatcher.close();
                cout << "¿Desea reintentar? (y/n)" << endl;
                char retry;
                cin >> retry;
                if (retry == 'Y' || retry == 'y')
                    continue;
                else
                    return;
            }
            else {
                cout << "Puerto vinculado, esperando clientes" << endl;

                break;
            }
        }
        IpAddress ip = IpAddress::LocalHost;
        cout << ip.getLocalAddress() << ":" << to_string(port) << endl;
        while (peerList.size() < MAX_PLAYERS) {
            TcpSocket* temp = new TcpSocket();
            if (dispatcher.accept(*temp) != Socket::Done)
            {
                cout << "Conexión no aceptada" << endl;
            }
            else {
                cout << "Conexión establecida con : " << temp << endl;
                cout << "   Ip : " << temp->getRemoteAddress() << endl;
                cout << "   Puerto : " << temp->getRemotePort() << endl;
                // Informar al socket de la información de la gente en la peerList
                MessageManager message = MessageManager(temp);
                if (!message.send_peers(&peerList)) {
                    cout << "Conexión perdida con : " << temp << endl;
                }
                peerList.push_back(Peer(temp->getRemoteAddress(), temp->getRemotePort()));
                temp->disconnect();
            }
        }
        dispatcher.close();
        cout << "Todos los jugadores conectados. Cerrando." << endl;
	}
};

#endif // BSS_INCLUDED
