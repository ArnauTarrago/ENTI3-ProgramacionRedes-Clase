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
