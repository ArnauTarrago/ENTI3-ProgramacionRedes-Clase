#include <thread>
#include <sstream>
#include <iostream>    
#include <mutex>
#include <chrono>
#include <string>
#include <list>
#include <tuple>

#define MAXPLAYERS 3

#pragma region OS INFO
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	//define something for Windows (32-bit and 64-bit, this part is common)
		#define OS_WINDOWS 1
	#ifdef _WIN64
	   //define something for Windows (64-bit only)
	#else
	   //define something for Windows (32-bit only)
	#endif
	#elif __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
	// iOS Simulator
	#elif TARGET_OS_IPHONE
	// iOS device
	#elif TARGET_OS_MAC
	// Other kinds of Mac OS
	#else
	#   error "Unknown Apple platform"
	#endif
	#elif __linux__
		// linux
		#define OS_LINUX 0
	#elif __unix__ // all unices not caught above
	// Unix
	#elif defined(_POSIX_VERSION)
	// POSIX
	#else
	#   error "Unknown compiler"
	#endif
#pragma endregion

#include "shared.h"
#include "ConsoleControl.h"
enum PROGRAMTYPE { PROGRAMTYPE_NOTSET, PROGRAMTYPE_CLIENT, PROGRAMTYPE_SERVER, PROGRAMTYPE_COUNT };
const static string PROGRAMTYPE_STRINGS[] = {
	"NOT SET",
	"CLIENT",
	"SERVER",
	"3",
};
PROGRAMTYPE programtype = PROGRAMTYPE::PROGRAMTYPE_NOTSET;
struct BSS;
struct Player;
void AddMessage(const string message, ConsoleColor color = WHITE, bool print = true);
void AddConnection(const TcpSocket* temp, bool ok);
void UpdateServer(BSS* server);
void UpdateClient(Player* player);
void FilterGamelist(vector<GameSessionSend> games, GameSessionFilter* filter, GameSessionOrder* order);
void PrintGamelist(const vector<GameSessionSend> games, GameSessionFilter* filter = nullptr, GameSessionOrder* order = nullptr);
bool GetInput_Confirmation(bool registerLine = true);
string GetInput_String(bool registerLine = true);
char GetInput_Char(bool registerLine = true);
int GetInput_Int(bool registerLine = true);
#include "BSS.h"
#include "Player.h"
#include "Interface.h"

/*
c
localhost
50000

*/


Interface ui;

void AddMessage(const string message, ConsoleColor color, bool print) { ui.AddMessage(message, color, print); }
void AddConnection(const TcpSocket* temp, bool ok) { ui.AddConnection(temp, ok); }
void UpdateServer(BSS* server) { ui.UpdateServer(server); }
void UpdateClient(Player* player) { ui.UpdateClient(player); }
void FilterGamelist(vector<GameSessionSend> games, GameSessionFilter* filter, GameSessionOrder* order) { ui.FilterGamelist(games, filter, order); }
void PrintGamelist(vector<GameSessionSend> games, GameSessionFilter* filter, GameSessionOrder* order) { ui.PrintGamelist(games, filter, order); }
bool GetInput_Confirmation(bool registerLine) { return ui.GetInput_Confirmation(registerLine); };
string GetInput_String(bool registerLine) { return ui.GetInput_String(registerLine); }
char GetInput_Char(bool registerLine) { return ui.GetInput_Char(registerLine); }
int GetInput_Int(bool registerLine) { return ui.GetInput_Int(registerLine); }

int main()
{
	ui.statusType.SetText(PROGRAMTYPE_STRINGS[programtype].c_str());
	ui.PrintScreen();

	ui.AddMessage("Are you a client 'c' or server 's'?:");
	char userType = ui.GetInput_Char();

	sf::TcpSocket* sock_0 = new sf::TcpSocket();
	bool okConnection = false;


	if (userType == 's')
	{
		programtype = PROGRAMTYPE::PROGRAMTYPE_SERVER;
		BSS bss;
	}
	else
	{
		programtype = PROGRAMTYPE::PROGRAMTYPE_CLIENT;
		Player player;
	}

#if OS_WINDOWS
	system("pause");
#elif OS_LINUX
	system("read");
#endif
	return 0;
}