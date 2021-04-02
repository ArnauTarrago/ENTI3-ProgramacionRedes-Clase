#ifndef CARD_INCLUDED
#define CARD_INCLUDED
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <map>
using namespace std;
struct Card {
	static enum CATEGORY
	{
		ARABE, BANTU, CHINA, ESQUIMAL, INDIA, MEXICANA, TIROLESA
		, CATEGORY_COUNT
	};
	inline static const char* ToString(CATEGORY v)
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
	inline static const char* ToString(NUMBER v)
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
	Card(const Card &_card) : CAT(_card.CAT), NUM(_card.NUM), val(_card.val) {}
	Card(CATEGORY _cat, NUMBER _num) : CAT(_cat), NUM(_num), val(CAT* CATEGORY_COUNT + NUM) {}
	Card(int _cat, int _num) : CAT(static_cast<CATEGORY>(_cat)), NUM(static_cast<NUMBER>(_num)), val(CAT* CATEGORY_COUNT + NUM) {}
	inline static const string ToString(Card c) {
		return " - (" + to_string(c.val) + ")	:" + ToString(c.CAT) + "(" + to_string(c.CAT) + "), " + ToString(c.NUM) + "(" + to_string(c.NUM) + ")";
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
		shuffle(deck.begin(), deck.end(), default_random_engine(_seed));
	}
	void Print() {
		cout << "Generated deck:" << endl;
		for (size_t i = 0; i < deck.size(); i++)
		{
			cout << Card::ToString(*deck.at(i)) << endl;
		}
		cout << endl;
	}
	inline const int size() {
		return deck.size();
	}
};
struct Hand {
	bool isActive;
	int currentTurn;
	map<int, int> categories;
	map<Card, bool> hand;
	Hand() {
		for (size_t i = 0; i < Card::CATEGORY_COUNT; i++)
		{
			categories[i] = 0;
		}
	}
	bool has(Card _card) {
		return hand[_card];
	}
	void add(Card _card) {
		categories[_card.CAT]++;
		hand[_card] = true;
	}
	void remove(Card _card) {
		categories[_card.CAT]--;
		hand[_card] = false;
	}
	void Print() {
		cout << "Current Points:" << endl;
		for (size_t i = 0; i < Card::CATEGORY_COUNT; i++)
		{
			cout << Card::ToString(static_cast<Card::CATEGORY>(i)) << ": " << categories[static_cast<Card::CATEGORY>(i)] << ", ";
		}
		cout << endl << "Points: " << points() << endl;
		cout << "Current Hand:" << endl;
		for (map<Card, bool>::iterator it = hand.begin(); it != hand.end(); ++it) {
			if (it->second)
				cout << Card::ToString(it->first) << endl;
		}
	}
	int points(){
		int point = 0;
		for (size_t i = 0; i < Card::CATEGORY_COUNT; i++)
		{
			if (categories[static_cast<Card::CATEGORY>(i)] >= Card::NUMBER_COUNT)
				point++;
		}
		return point;
	}
};
#endif // CARD_INCLUDED
