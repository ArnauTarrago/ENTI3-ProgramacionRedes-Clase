#ifndef BSS_INCLUDED
#define BSS_INCLUDED
#include <SFML/Network.hpp>
#include "shared.h"
using namespace std;
using namespace sf;
struct BSS {
    const int MAX_PLAYERS;
	TcpSocket* sock = new TcpSocket();
	vector<Peer> peerList = vector<Peer>();
	int port;
	Socket::Status status;
	TcpListener dispatcher;
	BSS(int _MAX_PLAYERS = 3) : MAX_PLAYERS(_MAX_PLAYERS){
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
                if (peerList.size() > 0)
                {
                    // Informar al socket de la información de la gente en la peerList
                    MessageManager message = MessageManager(temp);
                    if (!message.send_peers(&peerList)) {
                        cout << "Conexión perdida con : " << temp << endl;
                    }
                }
                peerList.push_back(Peer(temp->getRemoteAddress(), temp->getRemotePort()));
                temp->disconnect();
            }
        }
        dispatcher.close();
	}
};

#endif // BSS_INCLUDED
