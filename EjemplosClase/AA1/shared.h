#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#include <SFML/Network.hpp>
#include <iostream>

using namespace std;
using namespace sf;

struct Peer
{
    const IpAddress ip;
    const unsigned short port;

    Peer(const IpAddress _ip, const unsigned short _port) :ip(_ip), port(_port) {}
};
struct PeerComplete : Peer
{
    TcpSocket* socket;
    PeerComplete(const IpAddress _ip, const unsigned short _port) : Peer(_ip, _port), socket(nullptr) {}
    PeerComplete(const IpAddress _ip, const unsigned short _port, TcpSocket* _socket) : Peer(_ip, _port), socket(_socket) {}
    PeerComplete(TcpSocket* _socket) : Peer(_socket->getRemoteAddress(), _socket->getRemotePort()), socket(_socket) {}
};
enum COMUNICATION_MSGS
{
    MSG_NULL, MSG_OK, MSG_KO, MSG_PEERS_START, MSG_PEER, MSG_PEERS_END
    , MSG_COUNT
};

class MessageManager
{
public:
    PeerComplete peer;
    Packet lastPacket;
    Socket::Status lastStatus;
    bool connected = true;
    MessageManager(Peer _peer) :peer(_peer.ip, _peer.port) {}
    MessageManager(PeerComplete _peer) :peer(_peer.ip, _peer.port, _peer.socket) {}
    MessageManager(TcpSocket* _socket) :peer(_socket) {}

#pragma region FUNCIONES GENERALES
    Packet receive_message() {
        Packet pack;
        Socket::Status receiveStatus = this->peer.socket->receive(pack);
        this->lastStatus = receiveStatus;
        if (receiveStatus == Socket::Disconnected) {
            cout << "Desconectado" << endl;
            this->connected = false;
            this->peer.socket->disconnect();
        }
        if (receiveStatus != Socket::Done)
        {
            cout << "Recepción de datos fallida" << endl;
        }
        else {
            this->lastPacket = pack;
        }
        return pack;
    }
    bool send_message(Packet& packet_) {
        Socket::Status receiveStatus = this->peer.socket->send(packet_);
        this->lastStatus = receiveStatus;
        if (receiveStatus == Socket::Disconnected) {
            cout << "Desconectado" << endl;
            return false;
            this->connected = false;
            this->peer.socket->disconnect();
        }
        if (receiveStatus != Socket::Done)
        {
            return false;
            cout << "Envio de datos fallido" << endl;
        }
        else {
            return true;
        }
        return false;
    }
#pragma endregion

#pragma region ENVIOS ESPECIFICOS
    bool send_ok() {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_OK;
        return send_message(pack);
    }
    bool send_ko() {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_KO;
        return send_message(pack);
    }
    bool send_peers(const vector<Peer>* _peers) {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_PEERS_START;
        if (send_message(pack)) {
            for (size_t i = 0; i < _peers->size(); i++)
            {
                pack.clear();
                pack << COMUNICATION_MSGS::MSG_PEER << _peers->at(i).ip.toInteger() << _peers->at(i).port;
                if (!send_message(pack)) {
                    return false;
                }
            }
            pack.clear();
            pack << COMUNICATION_MSGS::MSG_PEERS_END;
            if (!send_message(pack)) {
                return false;
            }
        }
        else {
            return false;
        }
        return true;
    }
#pragma endregion
#pragma region RECEPCIONES ESPECIFICAS
    bool receive_peers(vector<Peer>* _peers) {
        Packet pack = receive_message();
        int msg = COMUNICATION_MSGS::MSG_NULL;
        if (pack >> msg && msg == COMUNICATION_MSGS::MSG_PEERS_START) {
            while (true) {
                pack.clear();
                pack = receive_message();
                if (pack >> msg) {
                    if (msg == COMUNICATION_MSGS::MSG_PEER) {
                        int intIPAddress;
                        IpAddress ipAddress;
                        unsigned short port;

                        if (pack >> intIPAddress >> port) {
                            ipAddress = IpAddress(intIPAddress);
                            _peers->push_back(Peer(ipAddress, port));
                        }
                        else {
                            return false;
                        }
                    }
                    else if (msg == COMUNICATION_MSGS::MSG_PEERS_END) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
            }
        }
        else {
            return false;
        }
        return false;
    }
#pragma endregion

};

#endif // SHARED_INCLUDED
