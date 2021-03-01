#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#include <SFML/Network.hpp>
#include <iostream>

using namespace std;

enum COMUNICATION_MSGS
{
    MSG_NULL, C_LOGIN, S_OK, S_KO, S_ROULETTE, C_ROULETTE, S_GEMPICKED, S_MAPNAME, C_MAPNAME, S_MAPSIZE, S_MAPOBSTACLES,
    S_ENEMY, S_PLAYERPOS, C_MOVE, S_PLAYERATTACK, S_ENEMYATTACK, S_REMOVEENEMY, S_GEMPOSITION, S_NEWUSER, S_EXITUSER, C_EXITMAP, C_SAVE, C_LOGOUT
};

class MessageManager
{
public:
    sf::Packet lastPacket;
    sf::Socket::Status lastStatus;
    bool connected = true;
    sf::TcpSocket *client;
    MessageManager() {};
    MessageManager(sf::TcpSocket *client_) : client(client_) {};
    sf::Packet receive_message();
    bool send_message(sf::Packet & packet_);

    bool send_ok();
    bool send_ko();

    bool receive_login(string & username_, string & password_);
    bool receive_roulette(bool & tryRoulette_);
    bool receive_map(int & index_);
};


bool MessageManager::send_ok(){
    sf::Packet pack;
    pack << COMUNICATION_MSGS::S_OK;
    return send_message(pack);
}
bool MessageManager::send_ko(){
    sf::Packet pack;
    pack << COMUNICATION_MSGS::S_KO;
    return send_message(pack);
}

bool MessageManager::receive_login(string & username_, string & password_){
    sf::Packet pack = receive_message();
    int msg = COMUNICATION_MSGS::MSG_NULL;
    if(pack >> msg){
        if(msg == COMUNICATION_MSGS::C_LOGIN && pack >> username_ >> password_)
            return true;
    }
    return false;
}
bool MessageManager::receive_roulette(bool & tryRoulette_){
    sf::Packet pack = receive_message();
    int msg = COMUNICATION_MSGS::MSG_NULL;
    if(pack >> msg){
        if(msg == COMUNICATION_MSGS::C_ROULETTE && pack >> tryRoulette_)
            return true;
    }
    return false;
}
bool MessageManager::receive_map(int & index_){
    sf::Packet pack = receive_message();
    int msg = COMUNICATION_MSGS::MSG_NULL;
    if(pack >> msg){
        if(msg == COMUNICATION_MSGS::C_MAPNAME && pack >> index_)
            return true;
    }
    return false;
}
sf::Packet MessageManager::receive_message(){
    sf::Packet pack;
    sf::Socket::Status receiveStatus = this->client->receive(pack);
    this->lastStatus = receiveStatus;
    if(receiveStatus == sf::Socket::Disconnected){
        cout << "Desconectado" << endl;
        this->connected = false;
        this->client->disconnect();
    }
    if (receiveStatus != sf::Socket::Done)
    {
        cout << "Recepción de datos fallida" << endl;
    }else{
        this->lastPacket = pack;
    }
    return pack;
}

#endif // SHARED_INCLUDED
