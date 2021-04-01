#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#include <SFML/Network.hpp>
#include <iostream>

using namespace std;
using namespace sf;

#define GameSessionSend std::tuple<string, int, int, bool>

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
    MSG_NULL, MSG_OK, MSG_KO, MSG_PEERS, MSG_GREET, MSG_CHAT, MSG_VALIDATION, MSG_GAMEQUERY, MSG_GAMELIST, MSG_TURNDONE, MSG_REQUESTCARD
    , MSG_GAMEOVER
    , MSG_COUNT
};
enum GAME_QUERY {
    GAME_SETUP, GAME_JOIN
    , GAME_COUNT
};


enum CHAT_MSGS {
    CHAT_HELLO, CHAT_GG, CHAT_HELP
    , CHAT_COUNT
};

const static string CHAT_MSGS_STRINGS[] = {
    "Hello!",
    "Good Game!",
    "Help!"
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
            //cout << "Desconectado" << endl;
            this->connected = false;
            this->peer.socket->disconnect();
        }
        if (receiveStatus != Socket::Done)
        {
            //cout << "Recepción de datos fallida" << endl;
        }
        else {
            //cout << "Mensaje recibido: " << endl << pack << endl;
            this->lastPacket = pack;
        }
        return pack;
    }
    bool send_message(Packet& packet_) {
        Socket::Status receiveStatus = this->peer.socket->send(packet_);
        this->lastStatus = receiveStatus;
        if (receiveStatus == Socket::Disconnected) {
            //cout << "Desconectado" << endl;
            return false;
            this->connected = false;
            this->peer.socket->disconnect();
        }
        if (receiveStatus != Socket::Done)
        {
            return false;
            //cout << "Envio de datos fallido" << endl;
        }
        else {
            //cout << "Mensaje enviado: " << endl << packet_ << endl;
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
    bool send_peers(const list<MessageManager*>* _peers) {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_PEERS;
        pack << static_cast<sf::Uint32>(_peers->size());
        for (list<MessageManager*>::const_iterator it = _peers->begin(); it != _peers->end(); it++) {
            pack << (*it)->peer.ip.toInteger() << (*it)->peer.port;
        }
        if (!send_message(pack)) {
            return false;
        }
        return true;
    }
    bool send_gamelist(const vector<GameSessionSend>* _games) {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_GAMELIST;
        pack << static_cast<sf::Uint32>(_games->size());
        for (size_t i = 0; i < _games->size(); i++)
        {
            pack << get<0>(_games->at(i)) << get<1>(_games->at(i)) << get<2>(_games->at(i)) << get<3>(_games->at(i));
        }
        if (!send_message(pack)) {
            return false;
        }
        return true;
    }
	bool send_gameQuery(bool isCreatingServer, string name, string password, int maxPlayers) {
		Packet pack;
		pack << COMUNICATION_MSGS::MSG_GAMEQUERY;
		if (isCreatingServer)
		{
			pack << GAME_QUERY::GAME_SETUP;
		}
		else
		{
			pack << GAME_QUERY::GAME_JOIN;
		}
		pack << name << password << maxPlayers;
		return send_message(pack);
	}
    bool send_greet(int _Id) {
        Packet pack;
        pack << COMUNICATION_MSGS::MSG_GREET << _Id;
        return send_message(pack);
    }
    //bool send_seed(int _seed) {
    //    Packet pack;
    //    pack << COMUNICATION_MSGS::MSG_SEED << _seed;
    //    return send_message(pack);
    //}
#pragma endregion
#pragma region RECEPCIONES ESPECIFICAS
    bool receive_peers(vector<Peer>* _peers) {
        Packet pack = receive_message();
        int msg = COMUNICATION_MSGS::MSG_NULL;
        if (pack >> msg && msg == COMUNICATION_MSGS::MSG_PEERS) {
            sf::Uint32 size = -1;
            if (pack >> size ) {
                _peers->reserve(size);
                for (size_t i = 0; i < size; i++)
                {
                    sf::Uint32 intIPAddress;
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
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
        return false;
    }
    bool receive_gamelist(vector<GameSessionSend>* _games) {
        Packet pack = receive_message();
        int msg = COMUNICATION_MSGS::MSG_NULL;
        if (pack >> msg && msg == COMUNICATION_MSGS::MSG_GAMELIST) {
            sf::Uint32 size = -1;
            if (pack >> size ) {
                _games->reserve(size);
                for (size_t i = 0; i < size; i++)
                {

                    string name;
                    int currentPlayers;
                    int maxPlayers;
                    bool hasPassword;

                    if (pack >> name >> currentPlayers >> maxPlayers >> hasPassword) {
                        _games->push_back(make_tuple(name, currentPlayers, maxPlayers, hasPassword));
                    }
                    else {
                        return false;
                    }
                }
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
        return false;
    }
    bool receive_greet(int * Id_) {
        Packet pack = receive_message();
        int msg = COMUNICATION_MSGS::MSG_NULL;
        if (pack >> msg && msg == COMUNICATION_MSGS::MSG_GREET) {
            int id = 0;
            if (pack >> id) {
                *Id_ = id;
                return true;
            }
        }
        return false;
    }

    bool receive_gameQuery(bool * create, string * name, string * password, int * maxPlayers) {
        Packet pack = receive_message();
        int msg = COMUNICATION_MSGS::MSG_NULL;
        if (pack >> msg && msg == COMUNICATION_MSGS::MSG_GAMEQUERY) {
            if (pack >> msg >> *name >> *password) {
                if (msg == GAME_QUERY::GAME_JOIN) {
                    *create = false;
                    return true;
                }
                else if (msg == GAME_QUERY::GAME_SETUP) {
                    *create = true;
                    if (pack >> *maxPlayers)
                        return true;
                }
            }
        }
        return false;
    }



    //bool receive_seed(int * seed_) {
    //    packet pack = receive_message();
    //    int msg = comunication_msgs::msg_null;
    //    if (pack >> msg && msg == comunication_msgs::msg_seed) {
    //        int seed = 0;
    //        if (pack >> seed) {
    //            *seed_ = seed;
    //            return true;
    //        }
    //    }
    //    return false;
    //}
#pragma endregion

};

#endif // SHARED_INCLUDED
