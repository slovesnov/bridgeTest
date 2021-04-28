/*
 * PreferansOld.h
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef PREFERANSOLD_H_
#define PREFERANSOLD_H_

#include <cstdint>
#include <cassert>
#include "BridgeConstants.h"
#ifndef CONSOLE
#include "../problem/Problem.h"
#endif

#ifndef FINAL_RELEASE
//Note. Not use NODE_COUNT macro because also have Bridge class with its own macro
//	#define PREFERANS_NODE_COUNT
#endif

class PreferansOld {
	static const int8_t HASH_EXACT = 0;
	static const int8_t HASH_ALPHA = 1;
	static const int8_t HASH_BETA = 2;
	static const int8_t HASH_INVALID = 3;
	int m_andKey;
	int m_hashSize;
	int m_trump, m_cards, m_depth;
	int m_code[4];

	struct Hash {
		int16_t code[3];
		int8_t f;
		int8_t v;
	}* m_hashTable;

	struct SC {
		int a[20];
		int length;
		SC() {
			length = 0;
		}

		inline void push(int v) {
			assert(length < 20);
			a[length++] = v;
		}

		inline void push(int v, int v1) {
			assert(length < 19);
			a[length++] = v;
			a[length++] = v1;
		}

		inline int operator[](int i) const {
			assert(i >= 0 && i < 20);
			return a[i];
		}
	};

	//BEGIN THIS IS AUTOMATICALLY GENERATED TEXT
	int e(int ww, int a, int b);
	int eb(int ww, int a, int b);
	int eMizer(int ww, int a, int b);
	int ebMizer(int ww, int a, int b);
	int e1(const int*w, int a, int b, int suit, int card, const SC& c1,
			const SC& c2);
	int e1b(const int*w, int a, int b, int suit, int card, const SC& c1,
			const SC& c2);
	int e1Mizer(const int*w, int a, int b, int suit, int card, const SC& c1,
			const SC& c2);
	int e1bMizer(const int*w, int a, int b, int suit, int card, const SC& c1,
			const SC& c2);
	int e2(const int*w, int a, int b, int suit, int card, int suit1, int card1,
			const SC& c2, int r1);
	int e2b(const int*w, int a, int b, int suit, int card, int suit1, int card1,
			const SC& c2, int r1);
	int e2Mizer(const int*w, int a, int b, int suit, int card, int suit1,
			int card1, const SC& c2, int r1);
	int e2bMizer(const int*w, int a, int b, int suit, int card, int suit1,
			int card1, const SC& c2, int r1);
	int f(int ww, int a, int b, int suit, int card, int suit1, int card1);
	int fb(int ww, int a, int b, int suit, int card, int suit1, int card1);
	int fMizer(int ww, int a, int b, int suit, int card, int suit1, int card1);
	int fbMizer(int ww, int a, int b, int suit, int card, int suit1, int card1);
	//END THIS IS AUTOMATICALLY GENERATED TEXT

	bool compare(int suit1, int card1, int suit2, int card2);

	void suitableCards(int suit, int w, SC& a);
	void suitableCards2P(int suit, const int* w, SC& c1, SC& c2);
	void suitableCards2(int suit, const int*w, SC&c1, SC&c2);
	void suitableCardsFromTrump(int suit, int w, SC& c);
	void suitableCardsAfterTrump(int suit, int w, SC& c);
	//all cards for one player
	void suitableCardsP(int suit, int w, SC& a);

	void removeCard(int suit, int pos);
	void restoreCard(int suit, int pos, int w);

#ifndef NDEBUG
	int getW(int suit, int pos);
#endif

	int hashIndex(int w);
	void recordHash(int v, int8_t f, int w);

public:

#ifdef PREFERANS_NODE_COUNT
	int m_nodes;
#endif

	int m_e; //estimate from player who do move
	int m_playerTricks; //estimate of player
	int m_best;

	/* if smallHash=true then one problem counts much faster 0.02 seconds
	 * but if smallHash=false the one problem counts about 0.3 seconds
	 * if smallHash=false then count solve_all_foe 184,756 positions much faster
	 */
	PreferansOld(bool smallHash = true);
	~PreferansOld();
	/*
	 * solveb - find estimate+best move for non mizer game
	 * solvebMizer - find estimate+best move for mizer game
	 * solve - find ESTIMATE ONLY for non mizer game
	 * solveMizer - find ESTIMATE ONLY for mizer game
	 *
	 * ptr [0-12 - spades A-2], [13-25 hearts A-2], [26-38 diamonds A-2], [39-51 clubs A-2]
	 * trump - like in bridge 0-spades, 1-hearts, 2-diamonds, 3-clubs, 4-NT
	 * firstmove
	 * player
	 * preferansPlayer for example {CARD_INDEX_NORTH,CARD_INDEX_EAST,CARD_INDEX_WEST };
	 */
	void solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solveMizer(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solvebMizer(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);

	//wrapper
	void solveEstimateOnly(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, bool mizer, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);

#ifndef CONSOLE
	void solve(const Problem& p, bool trumpChanged);
	void estimateAll(const Problem& p, ESTIMATE estimateType,
			SET_ESTIMATION_FUNCTION estimationFunction);
	void static suitableCards(int suit, const CARD_INDEX*cid,
			const CARD_INDEX next, SC&a);
#endif
};

#endif /* PREFERANSOLD_H_ */
