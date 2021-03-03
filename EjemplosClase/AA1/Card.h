#ifndef CARD_INCLUDED
#define CARD_INCLUDED
#include <vector>
#include <list>
#include <algorithm>
using namespace std;
struct Card {
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
	const CATEGORY CAT;
	const NUMBER NUM;
	Card() : CAT(CATEGORY_COUNT), NUM(NUMBER_COUNT) {}
	Card(CATEGORY _cat, NUMBER _num) : CAT(_cat), NUM(_num) {}
	Card(int _cat, int _num) : CAT(static_cast<CATEGORY>(_cat)), NUM(static_cast<NUMBER>(_num)) {}

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
	vector<Card> deck;
	Deck() {
		deck.reserve(Card::CATEGORY_COUNT * Card::NUMBER_COUNT);
		for (size_t i = 0; i < Card::CATEGORY::CATEGORY_COUNT; i++)
		{
			for (size_t j = 0; j < Card::NUMBER::NUMBER_COUNT; j++)
			{
				deck.push_back(Card(i, j));
			}
		}
	}
	void Shuffle(int seed) {
		std::random_shuffle(deck.begin(), deck.end(), seed);
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
