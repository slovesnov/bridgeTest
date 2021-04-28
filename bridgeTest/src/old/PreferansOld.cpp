/*
 * PreferansOld.cpp
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "PreferansOld.h"

PreferansOld::PreferansOld(bool smallHash) {
	int hashBits = smallHash ? 18 + 1 : 18 + 5;
	m_hashSize = 1 << hashBits;
	m_andKey = m_hashSize - 1;
	assert(hashBits>=18+1); //see hashIndex function
	m_hashTable = new Hash[m_hashSize];
}

PreferansOld::~PreferansOld() {
	delete[] m_hashTable;
}

void PreferansOld::removeCard(int suit, int pos) {
	pos <<= 1;
	int c = m_code[suit];
	m_code[suit] = ((c >> (pos + 2)) << pos) | (c & ((1 << pos) - 1));
}

void PreferansOld::restoreCard(int suit, int pos, int w) {
	pos <<= 1;
	int c = m_code[suit];
	m_code[suit] = ((c >> pos) << (pos + 2)) | (w << pos)
			| (c & ((1 << pos) - 1));
}

#ifndef NDEBUG
int PreferansOld::getW(int suit, int pos) {
	pos <<= 1;
	int c = m_code[suit];
	return (c >> pos) & 3;
}
#endif

bool PreferansOld::compare(int suit1, int card1, int suit2, int card2) {
	/*prevents gcc warnings for file pe.h with m_depth==1 may be used uninitialized
	 if (m_depth == 1) {
	 int card1, suit1, card2, suit2;
	 ....
	 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	return suit1 == suit2 ? card1 < card2 : suit2 != m_trump;
	//return suit1==suit2 ? card1>card2 : suit2!=m_trump;
#pragma GCC diagnostic pop
}

int PreferansOld::hashIndex(int w) {
	return ((m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w)
			& m_andKey;

//	return (m_code[0] ^ m_code[1] ^ (m_code[2] << 1) ^ (m_code[3] << 2) ^ w)
//			& m_andKey;
}

void PreferansOld::recordHash(int v, int8_t f, int w) {
	Hash& p = m_hashTable[hashIndex(w)];
	//code[3] is in index
	for (int i = 0; i < 3; i++) {
		//p.code[i]=m_code[i]&0xffff the same p.code[i]=m_code[i] tested for all possible m_code values
		p.code[i] = m_code[i];
		//p.code[i] = m_code[i] & 0xffff;
	}
	p.v = v;
	p.f = f;
}

//all cards for two players
void PreferansOld::suitableCards2P(int suit, const int* w, SC& c1, SC& c2) {
	suitableCards2(suit, w, c1, c2);
	if (c1.length > 0) {
		if (c2.length == 0) {
			suitableCardsFromTrump(suit, w[2], c2);
		}
		return;
	}
	else if (c2.length > 0) {
		suitableCardsFromTrump(suit, w[1], c1);
		return;
	}

	if (m_trump != NT && suit != m_trump) {
		suitableCards2(m_trump, w, c1, c2);
		if (c1.length > 0) {
			if (c2.length == 0) {
				suitableCardsAfterTrump(suit, w[2], c2);
			}
			return;
		}
		else if (c2.length > 0) {
			suitableCardsAfterTrump(suit, w[1], c1);
			return;
		}
	}

	for (int i = 0; i < 4; i++) {
		if (i != m_trump && i != suit) {
			suitableCards2(i, w, c1, c2);
		}
	}
}

//add cards in suit for two players
void PreferansOld::suitableCards2(int suit, const int* w, SC&c1, SC&c2) {
	int i, j, c;
	bool t1, t2;
	for (c = m_code[suit], j = 0, t1 = true, t2 = true; c != 3; j++, c >>= 2) {
		i = c & 3;
		if (i == w[1]) {
			t2 = true;
			if (t1) {
				c1.push(j);
				c1.push(suit);
				t1 = false;
			}
		}
		else if (i == w[2]) {
			t1 = true;
			if (t2) {
				c2.push(j);
				c2.push(suit);
				t2 = false;
			}
		}
		else {
			t1 = t2 = true;
		}
	}
}

void PreferansOld::suitableCardsFromTrump(int suit, int w, SC& c) {
	if (m_trump != NT && suit != m_trump) {
		suitableCards(m_trump, w, c);
	}
	if (c.length == 0) {
		suitableCardsAfterTrump(suit, w, c);
	}
}

void PreferansOld::suitableCardsAfterTrump(int suit, int w, SC& c) {
	for (int i = 0; i < 4; i++) {
		if (i != m_trump && i != suit) {
			suitableCards(i, w, c);
		}
	}
}

//add cards in suit for one player
void PreferansOld::suitableCards(int suit, int w, SC& a) {
	int j, c, f = 1;
	for (j = 0, c = m_code[suit]; c != 3; c >>= 2, j++) {
		if ((c & 3) == w) {
			if (f) {
				a.push(j);
				a.push(suit);
				f = 0;
			}
		}
		else {
			f = 1;
		}
	}
}

void PreferansOld::suitableCardsP(int suit, int w, SC& a) {
	int i;
	suitableCards(suit, w, a);
	if (a.length != 0) {
		return;
	}

	if (m_trump != NT && suit != m_trump) {
		suitableCards(m_trump, w, a);
	}

	if (a.length != 0) {
		return;
	}

	for (i = 0; i < 4; i++) {
		if (i != m_trump && i != suit) {
			suitableCards(i, w, a);
		}
	}
}

#ifndef CONSOLE
void PreferansOld::solve(const Problem& p, bool trumpChanged) {
	CARD_INDEX cid[52];
	p.getClearCid(cid);
	CARD_INDEX first = p.getFirst();
	if (p.m_mizer) {
		solvebMizer(cid, p.m_trump, first, p.m_player, p.m_preferansPlayer,
				trumpChanged);
	}
	else {
		solveb(cid, p.m_trump, first, p.m_player, p.m_preferansPlayer,
				trumpChanged);
	}
//	solve(cid, p.m_trump, first, p.m_player, p.m_mizer, p.m_preferansPlayer,
//			trumpChanged);
}
#endif

#ifndef CONSOLE
void PreferansOld::estimateAll(const Problem& pr, ESTIMATE estimateType,
		SET_ESTIMATION_FUNCTION estimationFunction) {
	const int best = m_best;
	Problem z = pr;
	State& st = z.getState();
	if (pr.isTableFull()) {
		st.clearInner();
	}
	int a = st.countInnerCards();
	CARD_INDEX* cid = st.m_cid;
	State so = st; //save cleared state

	CARD_INDEX next = pr.getNextMove();
	CARD_INDEX first = pr.getFirst();
	int i, j, suit = -1, e;
	CARD_INDEX l;
	const CARD_INDEX*p;
	for (i = 0; i < 4; i++) {
		p = cid + i * 13;
		for (j = 0; j < 8; j++) {
			l = *p++;
			if (l == first + CARD_INDEX_NORTH_INNER - CARD_INDEX_NORTH) {
				suit = i;
			}
		}
	}

	SC ca;
	if (suit == -1) {
		//any card
		for (i = 0; i < 4; i++) {
			suitableCards(i, cid, next, ca);
		}
	}
	else {
		m_trump = pr.m_trump;

		suitableCards(suit, cid, next, ca);
		if (ca.length == 0) {
			if (m_trump != NT && suit != m_trump) {
				suitableCards(m_trump, cid, next, ca);
			}

			if (ca.length == 0) {
				for (i = 0; i < 4; i++) {
					if (i != m_trump && i != suit) {
						suitableCards(i, cid, next, ca);
					}
				}
			}
		}
	}

	for (i = 0; i < ca.length; i += 2, st = so) {
		j = ca[i + 1] * 13 + ca[i];
		if (estimateType != ESTIMATE_ALL_LOCAL && estimateType != ESTIMATE_ALL_TOTAL
				&& j != best) {
			continue;
		}

		CARD_INDEX c = cid[j];
		bool pl = c == pr.m_player;
		cid[j] = Base::getInner(c);
		e = 0;
		bool empty = false;
		if (a == 2) {
			l = z.getNextMove();

			st.clearInner();
			/* in case of taker!=c find player which isn't l & not c
			 * if this player is m_player then c & l whist players
			 */
			if (l == c || (l != z.m_player && c != z.m_player)) {
				e = 1;
			}
			empty = st.countCards(z.m_player) == 0;
		}
		else {
			l = first;
		}
		if (!empty) {
			solveEstimateOnly(cid, pr.m_trump, l, pr.m_player, pr.m_mizer,
					pr.m_preferansPlayer, false);
			//m_cards-m_playerTricks = whist tricks
			e += pl ? m_playerTricks : m_cards - m_playerTricks;
		}
		estimationFunction(j, e);
	}
}
#endif

#ifndef CONSOLE
void PreferansOld::suitableCards(int suit, const CARD_INDEX*cid,
		const CARD_INDEX next, SC&a) {
	auto p = cid + suit * 13;
	for (int j = 0; j < 8; j++) {
		if (*p++ == next) {
			a.push(j, suit);
		}
	}
}
#endif

void PreferansOld::solveEstimateOnly(const CARD_INDEX c[52], int trump,
		CARD_INDEX first, CARD_INDEX player, bool mizer,
		const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
	if (mizer) {
		solveMizer(c, trump, first, player, preferansPlayer, trumpChanged);
	}
	else {
		solve(c, trump, first, player, preferansPlayer, trumpChanged);
	}
}

//BEGIN THIS IS AUTOMATICALLY GENERATED TEXT
int PreferansOld::e(int ww, int a, int b) {
#include "pe.h"
}
int PreferansOld::eb(int ww, int a, int b) {
#define STOREBEST
#include "pe.h"
#undef STOREBEST
}
int PreferansOld::eMizer(int ww, int a, int b) {
#define MIZER
#include "pe.h"
#undef MIZER
}
int PreferansOld::ebMizer(int ww, int a, int b) {
#define STOREBEST
#define MIZER
#include "pe.h"
#undef STOREBEST
#undef MIZER
}
int PreferansOld::e1(const int*w, int a, int b, int suit, int card, const SC& c1,
		const SC& c2) {
#include "pe1.h"
}
int PreferansOld::e1b(const int*w, int a, int b, int suit, int card, const SC& c1,
		const SC& c2) {
#define STOREBEST
#include "pe1.h"
#undef STOREBEST
}
int PreferansOld::e1Mizer(const int*w, int a, int b, int suit, int card,
		const SC& c1, const SC& c2) {
#define MIZER
#include "pe1.h"
#undef MIZER
}
int PreferansOld::e1bMizer(const int*w, int a, int b, int suit, int card,
		const SC& c1, const SC& c2) {
#define STOREBEST
#define MIZER
#include "pe1.h"
#undef STOREBEST
#undef MIZER
}
int PreferansOld::e2(const int*w, int a, int b, int suit, int card, int suit1,
		int card1, const SC& c2, int r1) {
#include "pe2.h"
}
int PreferansOld::e2b(const int*w, int a, int b, int suit, int card, int suit1,
		int card1, const SC& c2, int r1) {
#define STOREBEST
#include "pe2.h"
#undef STOREBEST
}
int PreferansOld::e2Mizer(const int*w, int a, int b, int suit, int card, int suit1,
		int card1, const SC& c2, int r1) {
#define MIZER
#include "pe2.h"
#undef MIZER
}
int PreferansOld::e2bMizer(const int*w, int a, int b, int suit, int card,
		int suit1, int card1, const SC& c2, int r1) {
#define STOREBEST
#define MIZER
#include "pe2.h"
#undef STOREBEST
#undef MIZER
}
int PreferansOld::f(int ww, int a, int b, int suit, int card, int suit1,
		int card1) {
#include "pf.h"
}
int PreferansOld::fb(int ww, int a, int b, int suit, int card, int suit1,
		int card1) {
#define STOREBEST
#include "pf.h"
#undef STOREBEST
}
int PreferansOld::fMizer(int ww, int a, int b, int suit, int card, int suit1,
		int card1) {
#define MIZER
#include "pf.h"
#undef MIZER
}
int PreferansOld::fbMizer(int ww, int a, int b, int suit, int card, int suit1,
		int card1) {
#define STOREBEST
#define MIZER
#include "pf.h"
#undef STOREBEST
#undef MIZER
}
void PreferansOld::solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#include "psolve.h"
}
void PreferansOld::solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define STOREBEST
#include "psolve.h"
#undef STOREBEST
}
void PreferansOld::solveMizer(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define MIZER
#include "psolve.h"
#undef MIZER
}
void PreferansOld::solvebMizer(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define STOREBEST
#define MIZER
#include "psolve.h"
#undef STOREBEST
#undef MIZER
}
//END THIS IS AUTOMATICALLY GENERATED TEXT

//	for(i=0;i<52;i++){
//		if(i%13==12){
//			g_print(" ");
//		}
//
//		if(i%13>=7){
//			continue;
//		}
//
//		g_print("%d",cid[i]);
//
//	}
