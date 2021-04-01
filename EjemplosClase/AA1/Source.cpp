#include <thread>
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
#include "BSS.h"
#include "Player.h"
#include "Interface.h"

/*
c
localhost
50000

*/

enum PROGRAMTYPE { CLIENT, SERVER, PROGRAMTYPE_COUNT };
const static string PROGRAMTYPE_STRINGS[] = {
	"CLIENT",
	"SERVER",
	"PROGRAMTYPE_COUNT",
};

int main()
{
	char userType;

	Interface interface;
	interface.SetGameState("Waiting for players");
	interface.SetCurrentPlayerTurn("Test 1");

	interface.AddMessage("1Mensaje");
	interface.AddMessage("2Mensaje");
	interface.AddMessage("3Mensaje");
	interface.AddMessage("4Mensaje");
	interface.AddMessage("5Mensaje");
	interface.AddMessage("6Mensaje");
	interface.AddMessage("7Mensaje");
	interface.AddMessage("asoihjgfhsadifuasieruf\n naweufh asdiu faoseb\n dfaisbef abesfiawefoiuasdfsdkjf nawipuerh\n fasidfn pawuiseh fawuefawsef");
	interface.AddMessage("asoihjgfhsadifuasierufnaweufhrgianiwesrfnoiasudfouiawbefouybawsefbawosuebfaukwsbefawsebdfuawbekifuawebfaukwebfwefafe");
	interface.AddMessage("asoihjgfhsadifuasierufnaweufhrgianiwesrfnoiasudfouiawbefouybawsefbawosuebfaukwsbefawsebdfuawbekifuawebfaukwebfwefafe");
	interface.AddMessage("asoihjgfhsadifuasierufnaweufhrgianiwesrfnoiasudfouiawbefouybawsefbawosuebfaukwsbefawsebdfuawbekifuawebfaukwebfwefafe");
	interface.AddMessage("asoihjgfhsadifuasierufnaweufhrgianiwesrfnoiasudfouiawbefouybawsefbawosuebfaukwsbefawsebdfuawbekifuawebfaukwebgre");
	interface.PrintMessages();

	string temp = interface.GetLine();

	ConsoleClear();


	std::cout << "Are you a client 'c' or server 's'?: ";
	std::cin >> userType;
	sf::TcpSocket* sock_0 = new sf::TcpSocket();
	bool okConnection = false;

	if (userType == 's')
	{
		BSS bss;
	}
	else if (userType == 'c')
	{
		Player player;
	}

#if OS_WINDOWS
	system("pause");
#elif OS_LINUX
	system("read");
#endif
	return 0;
}