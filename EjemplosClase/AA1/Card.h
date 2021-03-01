#ifndef CARD_INCLUDED
#define CARD_INCLUDED
#include <list>

#define Deck std::list<Card>

class Card {
public:
	static enum CATEGORY
	{
		ARABE, BANTU, CHINA, ESQUIMAL, INDIA, MEXICANA, TIROLESA
		, CATEGORY_COUNT
	};
	static enum NUMBER
	{
		ABUELO, ABUELA, PADRE, MADRE, HIJO, HIJA
		, NUMBER_COUNT
	};
	CATEGORY cat;
	NUMBER num;
};


#endif // CARD_INCLUDED
