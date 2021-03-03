#ifndef CARD_INCLUDED
#define CARD_INCLUDED
#include <vector>
#include <list>
#include <algorithm>
#include <random>
using namespace std;
struct Card {
	static enum CATEGORY
	{
		ARABE, BANTU, CHINA, ESQUIMAL, INDIA, MEXICANA, TIROLESA
		, CATEGORY_COUNT
	};
	inline const char* ToString(CATEGORY v)
	{
		switch (v)
		{
		case ARABE:		return "ARABE";
		case BANTU:		return "BANTU";
		case CHINA:		return "CHINA";
		case ESQUIMAL:	return "ESQUIMAL";
		case INDIA:		return "INDIA";
		case MEXICANA:	return "MEXICANA";
		case TIROLESA:	return "TIROLESA";
		default:		return "Unknown";
		}
	}
	static enum NUMBER
	{
		ABUELO, ABUELA, PADRE, MADRE, HIJO, HIJA
		, NUMBER_COUNT
	};
	inline const char* ToString(NUMBER v)
	{
		switch (v)
		{
		case ABUELO:	return "ABUELO";
		case ABUELA:	return "ABUELA";
		case PADRE:		return "PADRE";
		case MADRE:		return "MADRE";
		case HIJO:		return "HIJO";
		case HIJA:		return "HIJA";
		default:		return "Unknown";
		}
	}
	const CATEGORY CAT;
	const NUMBER NUM;
	const int val;
	Card() : CAT(CATEGORY_COUNT), NUM(NUMBER_COUNT), val(CAT* CATEGORY_COUNT + NUM) {}
	Card(const Card &_card) : CAT(_card.CAT), NUM(_card.NUM), val(_card.NUM) {}
	Card(CATEGORY _cat, NUMBER _num) : CAT(_cat), NUM(_num), val(CAT* CATEGORY_COUNT + NUM) {}
	Card(int _cat, int _num) : CAT(static_cast<CATEGORY>(_cat)), NUM(static_cast<NUMBER>(_num)), val(CAT* CATEGORY_COUNT + NUM) {}
	inline const string ToString() {
		return " - (" + to_string(val) + ")	:" + ToString(CAT) + "(" + to_string(CAT) + "), " + ToString(NUM) + "(" + to_string(NUM) + ")";
	}

	bool operator()(Card* a, Card* b) {
		return (a->CAT * CATEGORY_COUNT + a->NUM) > (b->CAT * CATEGORY_COUNT + b->NUM);
	}
};
bool operator== (const Card& a, const Card& b)
{
	return (a.CAT == b.CAT &&
		a.NUM == b.NUM);
}
bool operator!= (const Card& a, const Card& b)
{
	return !(a == b);
}
bool operator< (const Card& a, const Card& b)
{
	return (a.CAT * Card::CATEGORY_COUNT + a.NUM) < (b.CAT * Card::CATEGORY_COUNT + b.NUM);
}
bool operator> (const Card& a, const Card& b)
{
	return (a.CAT * Card::CATEGORY_COUNT + a.NUM) > (b.CAT * Card::CATEGORY_COUNT + b.NUM);
}
bool operator<= (const Card& a, const Card& b)
{
	return (a.CAT * Card::CATEGORY_COUNT + a.NUM) < (b.CAT * Card::CATEGORY_COUNT + b.NUM) || (a == b);
}
bool operator>= (const Card& a, const Card& b)
{
	return (a.CAT * Card::CATEGORY_COUNT + a.NUM) > (b.CAT * Card::CATEGORY_COUNT + b.NUM) || (a == b);
}

struct Deck {
	vector<Card*> deck;
	Deck() {
		deck.reserve(Card::CATEGORY_COUNT * Card::NUMBER_COUNT);
		for (size_t i = 0; i < Card::CATEGORY::CATEGORY_COUNT; i++)
		{
			for (size_t j = 0; j < Card::NUMBER::NUMBER_COUNT; j++)
			{
				deck.push_back(new Card(i, j));
			}
		}
	}
	void Shuffle(int _seed) {
		shuffle(deck.begin(), deck.end(), std::default_random_engine(_seed));
	}
	void Print() {
		cout << "Generated deck:" << endl;
		for (size_t i = 0; i < deck.size(); i++)
		{
			cout << deck.at(i)->ToString() << endl;
		}
		cout << endl;
	}
};
struct Hand {
	list<Card> hand;
	bool Has(Card _card) {
		return std::find(hand.begin(), hand.end(), _card) != hand.end();
	}
	bool Add(Card _card) {
		if (Has(_card))
			return false;
		hand.push_back(_card);
		return true;
	}
	bool remove(Card _card) {
		if (!Has(_card))
			return false;
		hand.remove(_card);
		return true;
	}
};
#endif // CARD_INCLUDED
