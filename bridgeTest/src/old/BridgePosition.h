/*
 * BridgePosition.h
 *
 *       Created on: 21.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef BRIDGEPOSITION_H_
#define BRIDGEPOSITION_H_

#include <cstddef> //NULL definition
#include "BridgeConstants.h"

#ifndef FINAL_RELEASE
/* preferans also have own node count macro so use BRIDGE_NODE_COUNT
 * instead of NODE_COUNT
 */
#define BRIDGE_NODE_COUNT
#endif

class BridgePosition {
	static const char HASH_ALPHA = 0;
	static const char HASH_BETA = 1;
	static const char HASH_UNKNOWN = 2;

	//for bf 272Mb
	static const unsigned SHIFT = 20;
	static const int HASH_ELEMENTS = 4;

	/*
	 !!!be careful with hashtable size because of half of code we cutting and shift >= 16 (necessarily)

	 number of elements in table=pow(2,shift+2)
	 size of one element = HashElem*sizeof(HASHITEM)+4 = 16*HashElem+4
	 size of memory for hashTable(MB)=pow(2,shift+2)*(16*HashElem+4)/1024/1024 = (4*HashElem+1)*pow(2,shift-16)

	 hashTableSize=272 Mb if shift=20 and HashElem=4

	 for increasing size of hashTable need increase shift or HashElem
	 */

	static const unsigned TRANSPOSITION_TABLE_SIZE = (1 << (SHIFT + 2));
	static const unsigned AND_KEY = TRANSPOSITION_TABLE_SIZE - 1;
	static const unsigned IMPOSSIBLE_KEY = 0xFFFFFFFFU;

	static const unsigned MAX_SURE_TRICKS_SEQUENCE = 10;
	static const unsigned MAX_SURE_TRICKS_SEQUENCE2 = (MAX_SURE_TRICKS_SEQUENCE
			<< 1);

	static unsigned startIndex;
	unsigned code[3];
	union {
		unsigned _code;
		struct {
			unsigned short _lowcode, _highcode;
		};
	};

	static int old_ptr[52];
	struct HASHITEM {
		unsigned short _code;
		char value;
		char flag;
		unsigned code[3];
	};

	struct HASHE {
		HASHITEM i[HASH_ELEMENTS];
		unsigned nextindex;
	};

	struct ListItem {
		union {
			struct {
				unsigned char card, suit;
			};
			unsigned short suitcard;
		};
		char who;
		ListItem*next, *prev;
		ListItem() {
			who = 0;
			next = prev = NULL;
		}

	}*p1, *p2, *p3, *suitable_t1[13], *suitable_t2[13], *suitable_t3[13];
	static ListItem d[4], q[4], t[4][13];
	//d[i]->t[i][0]->t[i][1]->...

	int whoTake(unsigned short sc, unsigned short sc1, unsigned short sc2,
			unsigned short sc3);
	inline int whoTake(unsigned short sc, ListItem*sc1, ListItem*sc2,
			ListItem*sc3) {
		return whoTake(sc, sc1->suitcard, sc2->suitcard, sc3->suitcard);
	}

	int est(unsigned short sc, unsigned short sc1, unsigned short sc2,
			unsigned short sc3);

	inline int est(unsigned short sc, ListItem*sc1, ListItem*sc2, ListItem*sc3) {
		return est(sc, sc1->suitcard, sc2->suitcard, sc3->suitcard);
	}
	inline int est(ListItem*sc, unsigned short sc1, ListItem*sc2, ListItem*sc3) {
		return est(sc->suitcard, sc1, sc2->suitcard, sc3->suitcard);
	}
	inline int est(ListItem*sc, ListItem*sc1, unsigned short sc2, ListItem*sc3) {
		return est(sc->suitcard, sc1->suitcard, sc2, sc3->suitcard);
	}
	inline int est(ListItem*sc, ListItem*sc1, ListItem*sc2, unsigned short sc3) {
		return est(sc->suitcard, sc1->suitcard, sc2->suitcard, sc3);
	}

	int est(unsigned short sc, unsigned short sc1, unsigned short sc2,
			unsigned short sc3, unsigned short qc, unsigned short qc1,
			unsigned short qc2, unsigned short qc3);

	int est(unsigned short sc, ListItem*sc1, ListItem*sc2, ListItem*sc3,
			unsigned short qc, ListItem*qc1, ListItem*qc2, ListItem*qc3) {
		return est(sc, sc1->suitcard, sc2->suitcard, sc3->suitcard, qc,
				qc1->suitcard, qc2->suitcard, qc3->suitcard);
	}

	//sureTricks
	static unsigned short *st[MAX_SURE_TRICKS_SEQUENCE + 1];

	//hashing
	static HASHE*hashtable;
	static HASHITEM*phash;
	static HASHE*ph;

	union {
		struct {
			unsigned _who_;
		};
		struct {
			char who, who1, who2, who3;
		};
	};
	static unsigned whoArray[79]; //array using for fast calculating WHO for child
	int whoIndex; //current index in array for element
public:

	static char trump;
	static bool trumpChanged;
private:
	int lastTrickValue();
	BridgePosition*next;

	void find_suitable_cards();
	unsigned counter_suitable_cards1, counter_suitable_cards2,
			counter_suitable_cards3;
	union {
		struct {
			unsigned char _card, _suit;
		};
		unsigned short _suitcard;
	};

	int depth;
	int gP(int alpha);
	int g(int alpha);

	int fg(int alpha);
	int fg1(int _alpha, BridgePosition::ListItem* _P0);
	int fg2(int _alpha, BridgePosition::ListItem* _P0,
			BridgePosition::ListItem* _P1);
	int fg3(int _alpha, BridgePosition::ListItem* _P0,
			BridgePosition::ListItem* _P1, ListItem* _P2);

	static bool hasSuit(int suit, int player, const int*cid);
public:
	static bool allocateTables();
	static void freeTables();
	static bool solveposition(const int ptr[52], char tr);

	/* this function estimate all turns
	 * __firstmove=0..3 - firstmove from previous state
	 */
	static void estimateAll(const int oldPtr[52], const int newPtr[52],
			char _trump, int _firstmove, ESTIMATE estimateType,
			SET_ESTIMATION_FUNCTION estimationFunction);
	static void switchMove(int*ptr);

	static short northSouthTricks;
	static short eastWestTricks;
	static char bs;
	static char bc;
	static int nextmove; //nextmove=0..3
	static int firstmove;
	static int guess;
	static int maximize;
#ifdef BRIDGE_NODE_COUNT
	static int nodeCount;
#endif

	/*
	 * bridge solver
	 * trumpChanged is used to clear hash table
	 */
	static bool solve(const int ptr[52], char _trump, int firstmove,
			bool trumpChanged);

	void printSuitableCards();
};

#endif /* BRIDGEPOSITION_H_ */
