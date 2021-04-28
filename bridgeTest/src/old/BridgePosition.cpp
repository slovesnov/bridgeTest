/*
 * BridgePosition.cpp
 *
 *       Created on: 21.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "BridgePosition.h"
#include <cstdio>
#include <cstdlib> //exit() function
#include "BridgeCommon.h" //for printSuitableCards()

unsigned short *BridgePosition::st[MAX_SURE_TRICKS_SEQUENCE + 1];
static int nofbits[4 * 8192]; //nofbits - number of bits in nofbits[suit<<13+card^1fff] times 2
static unsigned CodeArrayST[4 * 8192];

//not clear TRANSPOSITION TABLE.
#define USE_PREVIOUS

//Enhanced transposition cutoffs
#define ENHANCED

static unsigned _CodeArray[4 * 8192];
static unsigned SSCODE[4];

int LAST_BIT_CODE[8192];
int ENUMERATION_ARRAY[4096];

static BridgePosition P[14];

char BridgePosition::trump;
bool BridgePosition::trumpChanged;
char BridgePosition::bs;
char BridgePosition::bc;
short BridgePosition::northSouthTricks;
short BridgePosition::eastWestTricks;
int BridgePosition::nextmove = 0;
int BridgePosition::firstmove = 0;
int BridgePosition::guess;
int BridgePosition::maximize;
#ifdef BRIDGE_NODE_COUNT
int BridgePosition::nodeCount;
#endif
int BridgePosition::old_ptr[52];
BridgePosition::ListItem BridgePosition::d[4], BridgePosition::q[4];
BridgePosition::ListItem BridgePosition::t[4][13];
unsigned BridgePosition::whoArray[79];
unsigned BridgePosition::startIndex;
//hashing
BridgePosition::HASHE* BridgePosition::ph;
BridgePosition::HASHE* BridgePosition::hashtable;
BridgePosition::HASHITEM* BridgePosition::phash;
unsigned number_of_hash_goals;
unsigned number_of_collisions;

//this macro redefine lower using table of comparing
#define compare(suit1,card1,suit2,card2) (suit1==suit2?card1>card2:suit2!=trump)

#ifdef hashcol
long hash_records;
long records_collisions;
long number_of_alpha_cutoffs;
const unsigned long int ul=0;
#endif

#define Remove1(P)			if(P!=NULL){P->prev->next=P->next;P->next->prev=P->prev;}
#define Restore1(P)			if(P!=NULL){P->next->prev=P->prev->next=P;}

#ifdef CONSOLE
bool signalFileExists();
#endif

void BridgePosition::freeTables() {
	if (hashtable != NULL) {
		delete[] hashtable;
		hashtable = NULL;
	}

	//free memory
	for (unsigned i = 1; i <= MAX_SURE_TRICKS_SEQUENCE; i++)
		delete[] (st[i]);

}
bool BridgePosition::allocateTables() {
	BridgePosition::trump = NT;
	if (hashtable != NULL) {
		delete[] hashtable;
		hashtable = NULL;
	}
	hashtable = new HASHE[TRANSPOSITION_TABLE_SIZE];
	if (hashtable == NULL) {
		return false;
	}

	//BEGIN SURE TRICKS
	/*
	 ST[length_of_sequence][index_of_sequence]
	 go from low to high
	 t  (1-4     bits) trick by first item
	 b0 (5th     bit ) is passing to first  player possible             (denote as 'bs' in article)
	 b2 (6th     bit ) is passing to second player possible             (denote as 'bp' in article)
	 t0 (8th-11n bits) number of tricks of first player by second item  (denote as 'ts' in article)
	 t2 (12-16th bits) number of tricks of second player by second item  (denote as 'tp' in article)
	 */

#define m2(x) ((x)<<1)
#define m16(x) ((x)<<4)
	unsigned j, code, kp, c[4]; //c[0] - cards have first ...
	unsigned b[3], t2[3], t, len; //,l;
	int i, n, nn, k; //dont do as unsigned
	bool onlyOneBit; //,tb[4];//tb - trump bits

	unsigned short *PTR; //ST[0..]

	for (nn = 1; nn <= int(MAX_SURE_TRICKS_SEQUENCE); nn++) {
		len = 1 << m2(nn);
		st[nn] = new unsigned short[len]; //2^(2*nn)

		for (PTR = st[nn], j = 0; j < len; j++, PTR++) { //j - sequence of bits & index higher bits have junior
			for (i = 0; i < 4; i++)
				c[i] = 0;
			for (i = k = 0; i < nn; i++, k += 2)
				c[(j >> k) & 3]++;

			n = nn;

			code = j;
			t = t2[0] = t2[2] = b[0] = b[2] = 0;
			onlyOneBit = false;

			while (n > 0 && ((k = code & 3) & 1) == 0) {
				c[k]--;
				n--;
				code >>= 2;

				kp = k ^ 2; //partner
				if (c[kp] > 0) {
					b[k] = 1; //dont move k because it using in cycle
					if (onlyOneBit) {
						onlyOneBit = false;
						b[kp] = 0;
					}
					n--;
					c[kp]--;
					for (k = m2(n); k >= 0; k -= 2)
						if (((code >> k) & 3) == kp) {
							code = ((code >> (k + 2)) << k) | (code & ((1 << k) - 1));
							break;
						}
					t++;
				}
				else
					t2[k]++;

				//remove lowest cards form p1&p3
				for (i = 1; i < 4; i += 2) {
					if (c[i] > 0) {
						n--;
						c[i]--;
						for (k = m2(n); k >= 0; k -= 2)
							if (((code >> k) & 3) == unsigned(i)) {
								code = ((code >> (k + 2)) << k) | (code & ((1 << k) - 1));
								if (k == 0)
									onlyOneBit = true;
								break;
							}
					}
				}

			} //while(n>0)

			*PTR = (t | (b[0] << 4) | (b[2] << 5) | (t2[0] << 8) | (t2[2] << 12));
		} //for(j)

	} //for(nn)

#undef m2
#undef m16
	//END SURE TRICKS

	//fill last_bit_code array
	for (i = 0; i < 8192; i++) {
		for (k = 0; k < 20; k++)
			if (((i >> k) & 1) != 0)
				break;
		LAST_BIT_CODE[i ^ 0x1fff] = k;
	}

	//fill PEREBOR_ARRAY
	t = 0;
	ENUMERATION_ARRAY[t++] = 8191;

	int j1, j2, j3, j4, j5, j6;
	for (j = 0; j < 13; j++)
		ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j);

	for (j1 = 1; j1 < 13; j1++)
		for (j2 = 0; j2 <= j1 - 1; j2++)
			ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j1) ^ (1 << j2);

	for (j1 = 1; j1 < 13; j1++)
		for (j2 = 0; j2 <= j1 - 1; j2++)
			for (j3 = 0; j3 <= j2 - 1; j3++)
				ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j1) ^ (1 << j2) ^ (1 << j3);

	for (j1 = 1; j1 < 13; j1++)
		for (j2 = 0; j2 <= j1 - 1; j2++)
			for (j3 = 0; j3 <= j2 - 1; j3++)
				for (j4 = 0; j4 <= j3 - 1; j4++)
					ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j1) ^ (1 << j2) ^ (1 << j3)
							^ (1 << j4);

	for (j1 = 1; j1 < 13; j1++)
		for (j2 = 0; j2 <= j1 - 1; j2++)
			for (j3 = 0; j3 <= j2 - 1; j3++)
				for (j4 = 0; j4 <= j3 - 1; j4++)
					for (j5 = 0; j5 <= j4 - 1; j5++)
						ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j1) ^ (1 << j2) ^ (1 << j3)
								^ (1 << j4) ^ (1 << j5);

	//Curly braces avoid gcc 7.3.0 warning
	for (j1 = 1; j1 < 13; j1++) {
		for (j2 = 0; j2 <= j1 - 1; j2++) {
			for (j3 = 0; j3 <= j2 - 1; j3++) {
				for (j4 = 0; j4 <= j3 - 1; j4++) {
					for (j5 = 0; j5 <= j4 - 1; j5++) {
						for (j6 = 0; j6 <= j5 - 1; j6++) {
							ENUMERATION_ARRAY[t++] = 8191 ^ (1 << j1) ^ (1 << j2) ^ (1 << j3)
									^ (1 << j4) ^ (1 << j5) ^ (1 << j6);
						}
					}
				}
			}
		}
	}

	return true;
}

void BridgePosition::switchMove(int*ptr) {
	int i;
	char _who1 = (firstmove + 5) % 4 + 5;
	char _who2 = (firstmove + 6) % 4 + 5;
	char _who3 = (firstmove + 7) % 4 + 5;

	short st = -1, cd = -1, st1 = -1, cd1 = -1, st2 = -1, cd2 = -1, st3 = -1,
			cd3 = -1;
	for (i = 0; i < 52; ++i) {
		if (ptr[i] > 0 && ptr[i] < 5) {
		} //21jul2014[if(ptr[i]>0 && ptr[i]<5);] ->
		else if (ptr[i] == firstmove + 5) {
			st = i / 13;
			cd = 12 - i % 13;
		}
		else if (ptr[i] == _who1) {
			st1 = i / 13;
			cd1 = 12 - i % 13;
		}
		else if (ptr[i] == _who2) {
			st2 = i / 13;
			cd2 = 12 - i % 13;
		}
		else if (ptr[i] == _who3) {
			st3 = i / 13;
			cd3 = 12 - i % 13;
		}
	}
	if (st3 != -1) { //purify the cards
		if (compare(st, cd, st1,
				cd1) && compare(st, cd, st2, cd2) && compare(st, cd, st3, cd3)) {
		}
		else if (!compare(st, cd, st1,
				cd1) && compare(st1, cd1, st2, cd2) && compare(st1, cd1, st3, cd3)) {
			firstmove++;
			if (firstmove >= 4)
				firstmove -= 4;
		}
		else if (!compare(st, cd, st2,
				cd2) && compare(st2, cd2, st1, cd1) && compare(st2, cd2, st3, cd3)) {
			firstmove += 2;
			if (firstmove >= 4)
				firstmove -= 4;
		}
		else {
			firstmove += 3;
			if (firstmove >= 4)
				firstmove -= 4;
		}

		nextmove = firstmove;

	}
	else {
		if (st3 == -1 && st2 != -1)
			nextmove = (firstmove + 3) % 4;
		if (st2 == -1 && st1 != -1)
			nextmove = (firstmove + 2) % 4;
		if (st1 == -1 && st != -1)
			nextmove = (firstmove + 1) % 4;
		if (st == -1 && st3 != -1)
			nextmove = (firstmove) % 4;
	}

}

bool CompareTable[781 * 1024 + 781];

//show estimation of all possible turns
void BridgePosition::estimateAll(const int oldPtr[52], const int newPtr[52],
		char _trump, int _firstmove, ESTIMATE estimateType,
		SET_ESTIMATION_FUNCTION estimationFunction) {
	int i, j, k, l;

	//store all parameters
#define M(p1,p2) p1 _##p2=p2;
	M(short, northSouthTricks);
	M(short, eastWestTricks);
	M(char, bs);
	M(char, bc);
	M(int, nextmove);
#undef M

	int n[52];
	int suitableC[13]; //suitable cards
	int counterSuitableCards = 0;

	/*
	 firstmove - leading player
	 0 - north
	 1 - east
	 ...
	 */
	BridgePosition p;
	p._suitcard = 0; //prevents gcc warnings
	for (k = i = 0; i < 52; i++) {
		if (newPtr[i] > 4)
			k++;
		if (newPtr[i] == _firstmove + 5) {
			p._suit = i / 13;
			p._card = 12 - i % 13;
		}
	}

	const int cardsOnTable = k;

	if (cardsOnTable == 0)
		return;

	p.who = _firstmove;
	p.who1 = (p.who + 1) % 4;
	p.who2 = (p.who + 2) % 4;
	p.who3 = (p.who + 3) % 4;

	ListItem* CurPtr;

	if (cardsOnTable != 1) {
		//restore cards in a list
		//Begin ::InitList
		for (i = 0; i < 4; ++i) {
			d[i].next = &(t[i][0]);
			d[i].prev = &d[i];
			q[i].next = &q[i];
			q[i].who = d[i].who = -1;	  //dont remove!!!
		}
		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 12; ++j)
				t[i][j].next = &(t[i][j + 1]);

			t[i][0].prev = &(d[i]);
			for (j = 1; j <= 12; ++j)
				t[i][j].prev = &(t[i][j - 1]);

		}
		char _who = p.who;
		//ptr[i] is an indicator
		char _who1 = (_who + 5) % 4 + 5;
		char _who2 = (_who + 6) % 4 + 5;
		char _who3 = (_who + 7) % 4 + 5;
		//5aug2014 int  codes[4][13];
		//5aug2014 for(i=0;i<4;++i)for(j=0;j<13;j++)codes[i][j]=-1;
		ListItem*CurPtr, *NullPtr, *P0 = NULL, *P1 = NULL, *P2 = NULL;//31aug2014 comment out,*P3=NULL;
		for (i = 0; i < 4; ++i) {
			NullPtr = &(d[i]);
			CurPtr = NullPtr->next;
			//NullPtr=CurPtr;
			for (j = 12; j >= 0; --j) {	  //for "normal" initialization
				if (oldPtr[i * 13 + j] > 0 /*&& ptr[i*13+j]<5*/) {
					CurPtr->card = char(12 - j);
					if (oldPtr[i * 13 + j] >= 5) {
						if (oldPtr[i * 13 + j] == _who + 5)
							P0 = CurPtr;
						else if (oldPtr[i * 13 + j] == _who1)
							P1 = CurPtr;
						else if (oldPtr[i * 13 + j] == _who2)
							P2 = CurPtr;
						else if (oldPtr[i * 13 + j] == _who3) {
							//P3=CurPtr;//31aug2014 comment out
						}

						CurPtr->who = char(oldPtr[i * 13 + j]) - 5;
					}
					else {
						CurPtr->who = char(oldPtr[i * 13 + j]) - 1;
					}	  //#
					CurPtr->suit = char(i);
					//5aug2014  if(oldPtr[i*13+j]<5)codes[i][12-j]=CurPtr->who;
					NullPtr = CurPtr;
					CurPtr = CurPtr->next;
				}
			}
			NullPtr->next = &(q[i]);
			q[i].prev = NullPtr;
			//CurPtr=NULL;
		}	  //for
				///end of InitList

		Restore1(P0);
		Restore1(P1);
		Restore1(P2);

		p.find_suitable_cards();

		//delete cards from list
		Remove1(P0);
		Remove1(P1);
		Remove1(P2);

	}

	if (cardsOnTable == 1) {
		//find leading card
		counterSuitableCards = 0;
		int _suit;
		for (_suit = 0; _suit < 4; ++_suit) {
			CurPtr = &q[_suit];
			while ((CurPtr = CurPtr->prev) != &d[_suit]) {
				/*if(CurPtr->who==p.who)
				 SuitableFirst[counterSuitableCards++]=CurPtr;*/
				if (CurPtr->who == p.who)
					suitableC[counterSuitableCards++] = CurPtr->suit * 13 + 12
							- CurPtr->card;

				while (CurPtr->who == p.who)
					CurPtr = CurPtr->prev;
			}	  //while
		}

	}
	else if (cardsOnTable == 2) {
		counterSuitableCards = p.counter_suitable_cards1;
		for (i = 0; i < counterSuitableCards; i++)
			suitableC[i] = p.suitable_t1[i]->suit * 13 + 12 - p.suitable_t1[i]->card;
	}
	else if (cardsOnTable == 3) {
		counterSuitableCards = p.counter_suitable_cards2;
		for (i = 0; i < counterSuitableCards; i++)
			suitableC[i] = p.suitable_t2[i]->suit * 13 + 12 - p.suitable_t2[i]->card;
	}
	else if (cardsOnTable == 4) {
		counterSuitableCards = p.counter_suitable_cards3;
		for (i = 0; i < counterSuitableCards; i++)
			suitableC[i] = p.suitable_t3[i]->suit * 13 + 12 - p.suitable_t3[i]->card;
	}
	else {
		//ASSERT(0);
	}

	//this is previous estimation! Dont insert it in other cycle
	//show estimation of best turn as well
	int best;
	for (i = 0; i < counterSuitableCards; i++) {
		k = suitableC[i];
		if (k == _bs * 13 + 12 - _bc)
			best =
					oldPtr[k] == 1 || oldPtr[k] == 3 ?
							_northSouthTricks : _eastWestTricks;
		else
			best = UNKNOWN_ESTIMATE;

		if (best != UNKNOWN_ESTIMATE
				|| (best == UNKNOWN_ESTIMATE
						&& (estimateType == ESTIMATE_ALL_LOCAL
								|| estimateType == ESTIMATE_ALL_TOTAL))) {
			estimationFunction(k, best);

			for (l = 0; l < 2; l++)
				for (j = k + 1 - 2 * l; j % 13 != l * 12 && j < 52 && j >= 0;
						j += 1 - 2 * l) {
					if (oldPtr[j] == 0)
						continue;
					if (oldPtr[j] != oldPtr[k])
						break;

					estimationFunction(j, best);
				}
		}  //if
	}  //for(i)

	if (estimateType == ESTIMATE_ALL_LOCAL || estimateType == ESTIMATE_ALL_TOTAL)
		for (i = 0; i < counterSuitableCards; i++) {
			for (int ii = 0; ii < 52; ii++)
				n[ii] = oldPtr[ii];

			firstmove = _firstmove;
			nextmove = _nextmove;
			k = suitableC[i];

			if (k == _bs * 13 + 12 - _bc)  //don't need to count
				continue;

			n[k] += 4;

			//BEGIN purify cards
			int i, j;
			for (j = i = 0; i < 52; i++)
				if (n[i] > 4)
					j++;
			const int nCardsOnTable = j;

			if (nCardsOnTable == 4) {

				/*find firstmove & nextmove */
				int lt[4] = { -1, -1, -1, -1 };  //last cards
				int ind[4] = {
						firstmove % 4,
						(firstmove + 1) % 4,
						(firstmove + 2) % 4,
						(firstmove + 3) % 4 };

				for (i = 0; i < 52; i++)
					if (n[i] > 4) {
						for (j = 0; j < 4; j++)
							if (n[i] == 5 + ind[j]) {
								lt[j] = i;
								break;
							}

						n[i] = 0;
					}

#define compareS(sc1,sc2) (sc1/13==sc2/13 ? (12-sc1)>(12-sc2) : sc2/13!=trump)
				if (compareS(lt[0], lt[1])) {
					if (compareS(lt[0], lt[2])) {
						if (compareS(lt[0], lt[3]))
							i = 0;
						else
							i = 3;
					}
					else {
						if (compareS(lt[2], lt[3]))
							i = 2;
						else
							i = 3;
					}

				}
				else {  //take1
					if (compareS(lt[1], lt[2])) {
						if (compareS(lt[1], lt[3]))
							i = 1;
						else
							i = 3;

					}
					else {
						if (compareS(lt[2], lt[3]))
							i = 2;
						else
							i = 3;
					}
				}
				firstmove = nextmove = ind[i];

#undef compareS

			}
			//END purify cards

			switchMove(n);

			//whether need to add extra trick
			int addExtraTrick = 0;

			//nextmove=0..3 oldPtr=1..4
			if (nCardsOnTable == 4 && nextmove % 2 != oldPtr[k] % 2)
				addExtraTrick++;

			solveposition(n, _trump);
			best = oldPtr[k] % 2 == 1 ? northSouthTricks : eastWestTricks;
			best += addExtraTrick;

			estimationFunction(k, best);

			//sequences
			for (l = 0; l < 2; l++)
				for (j = k + 1 - 2 * l; j % 13 != l * 12 && j < 52 && j >= 0;
						j += 1 - 2 * l) {
					if (oldPtr[j] == 0)
						continue;
					if (oldPtr[j] != oldPtr[k])
						break;
					best =
							oldPtr[j] == 1 || oldPtr[j] == 3 ?
									northSouthTricks : eastWestTricks;
					estimationFunction(j, best);
				}

			n[k] -= 4;

		}  //for

		//restore all parameters
#define M(p1,p2) p2=_##p2;
	M(short, northSouthTricks);
	M(short, eastWestTricks);
	M(char, bs);
	M(char, bc);
	M(int, nextmove);
#undef M
	firstmove = _firstmove;
}

bool BridgePosition::solveposition(const int ptr[52], char tr) {
	if (hashtable == NULL) {
		return false;
	}

	number_of_hash_goals = number_of_collisions = 0;
	unsigned i = 0;
	int k, j;

	//[suit,12-card]<->ptr[suit*13+   card]
	//[suit,   card]<->ptr[suit*13+12-card]

	//begin code hashing
	for (i = 0; i < 4; ++i)
		SSCODE[i] = (i << 13);
	//end code hashing

	char _who = firstmove;

	i = 0, j = 0, k = 0;
#ifdef USE_PREVIOUS
	k = 0;
	for (i = 0; i < 52; ++i) {
		if (ptr[i] >= 1 && ptr[i] <= 4 && old_ptr[i] >= 1 && old_ptr[i] <= 4
				&& ptr[i] != old_ptr[i]) {
			k = 1;	//need clear transposition table
			break;
		}
	}
	if (BridgePosition::trumpChanged || k == 1) {
#endif
		for (i = 0; i < TRANSPOSITION_TABLE_SIZE; ++i) {
			hashtable[i].nextindex = 0;
			for (k = 0; k < HASH_ELEMENTS; ++k) {
				hashtable[i].i[k].code[0] = IMPOSSIBLE_KEY;	//don't need to fill for all
			}
		}
		BridgePosition::trump = tr;
		BridgePosition::trumpChanged = false;

#ifdef USE_PREVIOUS
	}
	for (i = 0; i < 52; ++i)
		old_ptr[i] = ptr[i];

#endif

	//fill table of comparison
	int s1, c1, s2, c2;
	for (s1 = 0; s1 < 4; s1++)
		for (s2 = 0; s2 < 4; ++s2)
			for (c1 = 0; c1 < 13; c1++)
				for (c2 = 0; c2 < 13; c2++) {
					//CompareTable[(s1<<8)+c1][(s2<<8)+c2]=compare(s1,c1,s2,c2);
					CompareTable[(((s1 << 8) + c1) << 10) + (s2 << 8) + c2] = compare(s1,
							c1, s2, c2);
				}

	//Begin ::InitList
	for (i = 0; i < 4; ++i) {
		d[i].next = &(t[i][0]);
		d[i].prev = &d[i];
		q[i].next = &q[i];
		q[i].who = d[i].who = -1;	//dont remove!!!
	}
	for (i = 0; i < 4; ++i) {
		/*for(j=0;j<12;++j)
		 T[i][j].Next=&(T[i][j+1]);

		 T[i][0].Prev=&(D[i]);

		 for(j=1;j<=12;++j)
		 T[i][j].Prev=&(T[i][j-1]);*/

		t[i][0].prev = &(d[i]);

		for (j = 0; j < 13; ++j) {
			if (j > 0)
				t[i][j].prev = &(t[i][j - 1]);
			if (j < 12)
				t[i][j].next = &(t[i][j + 1]);
		}

	}

	BridgePosition::startIndex = 0;

	//ptr[i] is an indicator
	char _who1 = (_who + 5) % 4 + 5;
	char _who2 = (_who + 6) % 4 + 5;
	char _who3 = (_who + 7) % 4 + 5;
	int codes[4][13];
	for (i = 0; i < 4; ++i)
		for (j = 0; j < 13; j++)
			codes[i][j] = -1;
	ListItem*CurPtr, *NullPtr, *P0 = NULL, *P1 = NULL, *P2 = NULL;//5aug2014 ,*P3=NULL;
	for (i = 0; i < 4; ++i) {
		NullPtr = &(d[i]);
		CurPtr = NullPtr->next;
		//NullPtr=CurPtr;
		for (j = 12; j >= 0; --j) {	//for "normal" initialization
			if (ptr[i * 13 + j] > 0 /*&& ptr[i*13+j]<5*/) {
				CurPtr->card = char(12 - j);
				if (ptr[i * 13 + j] >= 5) {
					if (ptr[i * 13 + j] == _who + 5)
						P0 = CurPtr;
					else if (ptr[i * 13 + j] == _who1)
						P1 = CurPtr;
					else if (ptr[i * 13 + j] == _who2)
						P2 = CurPtr;
					else if (ptr[i * 13 + j] == _who3) {
						//5aug 2014 P3=CurPtr;
					}
					CurPtr->who = char(ptr[i * 13 + j]) - 5;
				}
				else {
					CurPtr->who = char(ptr[i * 13 + j]) - 1;
				}	//#
				CurPtr->suit = char(i);
				if (ptr[i * 13 + j] < 5)
					codes[i][12 - j] = CurPtr->who;
				NullPtr = CurPtr;
				CurPtr = CurPtr->next;
			}
		}
		NullPtr->next = &(q[i]);
		q[i].prev = NullPtr;
		//CurPtr=NULL;
	}	//for
		///end of InitList
		//begin fill array of codes
	for (i = 0; i < 4; i++) {
		_CodeArray[(i << 13) + 8191] = 0;
		nofbits[(i << 13) + 8191] = 0;
	}

	for (i = 0; i < 4; i++) {
		for (j = 1; j < 8192; j++) {
			if (j >= 4096) {
				k = (ENUMERATION_ARRAY[8191 ^ j] ^ 0x1fff);
			}
			else
				k = ENUMERATION_ARRAY[j];
			if (codes[i][LAST_BIT_CODE[k]] == -1) {
				nofbits[(i << 13) + k] = nofbits[(i << 13) + (k | (k + 1))];
				_CodeArray[(i << 13) + k] = _CodeArray[(i << 13) + (k | (k + 1))];
			}
			else {
				nofbits[(i << 13) + k] = nofbits[(i << 13) + (k | (k + 1))] + 2;
				_CodeArray[(i << 13) + k] = _CodeArray[(i << 13) + (k | (k + 1))]
						| (codes[i][LAST_BIT_CODE[k]] << (nofbits[(i << 13) + k] - 2));
			}
		}
	}

	for (k = 0; k < 4; k++) {
		for (i = 0; i < 8192; ++i) {
			j = nofbits[(k << 13) + i];
			CodeArrayST[(k << 13) + i] = _CodeArray[(k << 13) + i];
			_CodeArray[(k << 13) + i] |= (13 - (j >> 1)) << j;
		}
	}

#ifdef hashcol
	number_of_collisions=number_of_hash_goals=hash_records=0;
	records_collisions=number_of_alpha_cutoffs=0;
#endif

#ifdef BRIDGE_NODE_COUNT
	nodeCount = 0;
#endif

	char st = -1, cd = -1, st1 = -1, cd1 = -1, st2 = -1, cd2 = -1, st3 = -1,
			cd3 = -1;
	for (i = 0, k = 0; i < 52; ++i) {
		if (ptr[i] > 0 && ptr[i] < 5)
			k++;
		else if (ptr[i] == _who + 5) {
			st = i / 13;
			cd = 12 - i % 13;
			k++;
		}
		else if (ptr[i] == _who1) {
			st1 = i / 13;
			cd1 = 12 - i % 13;
			k++;
		}
		else if (ptr[i] == _who2) {
			st2 = i / 13;
			cd2 = 12 - i % 13;
			k++;
		}
		else if (ptr[i] == _who3) {
			st3 = i / 13;
			cd3 = 12 - i % 13;
			k++;
		}
	}
	k /= 4;
	int n_cards = k;
	for (i = 1; i < 14; ++i) {
		P[i].next = &P[i - 1];
		P[i].depth = i;
	}
	if (st3 != -1) {
		if (compare(st, cd, st1,
				cd1) && compare(st, cd, st2, cd2) && compare(st, cd, st3, cd3)) {
		}
		else if (compare(st1, cd1, st,
				cd) && compare(st1, cd1, st2, cd2) && compare(st1, cd1, st3, cd3)) {
			_who++;
			if (_who >= 4)
				_who -= 4;
		}
		else if (compare(st2, cd2, st,
				cd) && compare(st2, cd2, st1, cd1) && compare(st2, cd2, st3, cd3)) {
			_who += 2;
			if (_who >= 4)
				_who -= 4;
		}
		else {
			_who += 3;
			if (_who >= 4)
				_who -= 4;
		}
	}
	P[k].whoIndex = 39;
	for (i = 0; i < 4; i++) {
		P[i].who = (_who + i) % 4;
		P[i].who1 = (_who + 1 + i) % 4;
		P[i].who2 = (_who + 2 + i) % 4;
		P[i].who3 = (_who + 3 + i) % 4;
		for (j = (i + 3) % 4; j < 79; j += 4)
			whoArray[j] = P[i]._who_;
	}
	P[k].who = (_who) % 4;
	P[k].who1 = (_who + 1) % 4;
	P[k].who2 = (_who + 2) % 4;
	P[k].who3 = (_who + 3) % 4;

	P[1].next = NULL;

	bool eastwest = true;
	if (firstmove == 0 || firstmove == 2)
		eastwest = false;

	if (st2 != -1)
		eastwest = !eastwest;
	else if (st1 != -1) {	//dont remove!!!
	}
	else if (st != -1)
		eastwest = !eastwest;

	int alpha;
	int low = 0, high = k + 1, goal, _bs = -1, _bc = -1;
	if (guess != 0 && !maximize) {
		//guess=1 0 tricks
		//...
		//guess=n n-1 trick(s)
		//...
		//if N tricks alpha=2n-15, beta=2n-13
		alpha = 2 * (guess - 1) - k - 1;

		if (st2 != -1) {
			eastwest = !eastwest;
			j = P[k].fg3(alpha, P0, P1, P2);
		}
		else if (st1 != -1) {
			j = P[k].fg2(alpha, P0, P1);
		}
		else if (st != -1) {
			eastwest = !eastwest;
			j = P[k].fg1(alpha, P0);
		}
		else {
			j = P[k].fg(alpha);
		}
		if (j <= alpha && guess != 0) {
			return false;
		}
		low = (n_cards + j) / 2;

	}
	//without lower_bound or with lower_bound and maximization
	else {
		bool first_time = true;
		if (guess != 0)
			low = guess - 3;

		//begin zero window search
		while (low + 1 < high) {
			goal = (low + high) / 2;

			alpha = 2 * goal - k - 2;

			if (st2 != -1) {
				j = P[k].fg3(alpha, P0, P1, P2);
			}
			else if (st1 != -1) {
				j = P[k].fg2(alpha, P0, P1);
			}
			else if (st != -1) {
				j = P[k].fg1(alpha, P0);
			}
			else {
				j = P[k].fg(alpha);
			}
			///
			if (j >= alpha + 2) {
				low = goal;
				_bs = bs;
				_bc = bc;
			}
			else {
				if (_bs == -1) {
					_bs = bs;
					_bc = bc;
				}
				high = goal;
			}
			if (guess != 0 && first_time) {
				first_time = false;
				if (j <= alpha) {
					return false;
				}
			}
		}
		bs = _bs;
		bc = _bc;
		//end zero window search*/
	}

	if (eastwest) {
		BridgePosition::eastWestTricks = low;
		BridgePosition::northSouthTricks = n_cards - low;
	}
	else {
		BridgePosition::eastWestTricks = n_cards - low;
		BridgePosition::northSouthTricks = low;
	}

	return true;
}
#undef compare
#define compare(suitcard1,suitcard2)	CompareTable[(suitcard1<<10)+suitcard2]

////////////////////////////////////////////////////////
#define KeyZob ((code[0]*code[1]+code[2])^_lowcode)

#define RecordHash(val,hashf,w)\
	ph=&(hashtable[(KeyZob^(w<<SHIFT)) & AND_KEY]);\
	phash=&(ph->i[ph->nextindex]);\
	phash->code[0]	=	code[0];\
	phash->code[1]	=	code[1];\
	phash->code[2]	=	code[2];\
	phash->_code		=	_highcode;\
	phash->value		=	val;\
	phash->flag			= hashf;\
	if(ph->nextindex == HASH_ELEMENTS-1)ph->nextindex=0;\
	else ph->nextindex++;

#define Remove(P)			P->prev->next=P->next;P->next->prev=P->prev;SSCODE[P->suit]^=(1<<P->card);
#define Restore(P)		P->next->prev=P->prev->next=P;SSCODE[P->suit]^=(1<<P->card);

int BridgePosition::whoTake(unsigned short sc, unsigned short sc1,
		unsigned short sc2, unsigned short sc3) {
	if (compare(sc, sc1)) {
		if (compare(sc, sc2)) {
			return compare(sc,sc3)?0:3;
		}
		return compare(sc2,sc3)?2:3;
	}
	if(compare(sc1,sc2)) {
		return compare(sc1,sc3)?1:3;
	}
	return compare(sc2,sc3)?2:3;
}

int BridgePosition::est(unsigned short sc, unsigned short sc1,
		unsigned short sc2, unsigned short sc3) {
	if (compare(sc, sc1)) {
		if (compare(sc, sc2)) {
			return compare(sc,sc3)?1:-1;
		}
		return compare(sc2,sc3)?1:-1;
	}
	if(compare(sc1,sc2)) {
		return -1;
	}
	return compare(sc2,sc3)?1:-1;

}

/*
 estimation for fixed sequence of cards
 */
int BridgePosition::est(unsigned short sc, unsigned short sc1,
		unsigned short sc2, unsigned short sc3, unsigned short qc,
		unsigned short qc1, unsigned short qc2, unsigned short qc3) {
	int j1 = whoTake(sc, sc1, sc2, sc3);

	if (j1 == 0)
		return est(qc, qc1, qc2, qc3) + 1;
	else if (j1 == 1)
		return (est(qc1, qc2, qc3, qc) == 1 ? -2 : 0);
	else if (j1 == 2)
		return est(qc2, qc3, qc, qc1) + 1;
	return (est(qc3, qc, qc1, qc2) == 1 ? -2 : 0);
}

bool First = true;

int BridgePosition::gP(int alpha) {
#ifdef BRIDGE_NODE_COUNT
	nodeCount++;
#endif
	if (alpha >= depth)
		return alpha;
	if (alpha + 2 <= -depth)
		return alpha + 2;

	ListItem*CurPtr;
	unsigned j1;

	if (next == NULL) {
#ifdef CONSOLE
		//this needs only for console application
		if(signalFileExists()) {
			BridgePosition::freeTables();
			exit(BRIDGE_CONSOLE_STATUS_USER_BREAK);
		}
#endif
		for (j1 = 0; j1 < 4; ++j1) {
			CurPtr = &q[j1];
			while ((CurPtr = CurPtr->prev) != &d[j1]) {
				if (CurPtr->who == who)
					_suitcard = CurPtr->suitcard;
				else if (CurPtr->who == who1)
					p1 = CurPtr;
				else if (CurPtr->who == who2)
					p2 = CurPtr;
				else
					p3 = CurPtr;
			}
		}
		if ((compare(_suitcard,p1->suitcard)&&
		compare(_suitcard,p2->suitcard) &&
		compare(_suitcard,p3->suitcard)) ||
		(!compare(_suitcard,p2->suitcard) &&
		compare(p2->suitcard,p1->suitcard) &&
		compare(p2->suitcard,p3->suitcard)))return +1;
		return -1;
	}

	////begin code generation
	unsigned*ptr = code;
	for (j1 = 0; j1 < 3; ++j1, ptr++) {
		*ptr = _CodeArray[SSCODE[j1]];
	}
	_code = _CodeArray[SSCODE[3]];
	////

//29nov2020 add options use QUICK_TRICK SURE_TRICKS using #define
#define QUICK_TRICK
#define SURE_TRICKS

#ifdef SURE_TRICKS
	//BEGIN_SURE_TRICKS
	unsigned j, j2;
	int sure = 0, sure0 = 0, sure2 = 0;
	unsigned SC, m0 = 0, s0, s2, min02 = 14;
	unsigned short ccode;
	int b0 = 0, b2 = 0;

	if (trump == NT) {
		for (j = 0; j < 4; ++j) {
			j2 = nofbits[SSCODE[j]];
			j1 = 0x1555555 >> (26 - j2);

			if (j2 == 0 || j2 > MAX_SURE_TRICKS_SEQUENCE2)
				continue;

			SC = CodeArrayST[SSCODE[j]];

			if (who == 2)
				SC ^= (0x2aaaaaa >> (26 - j2));
			else if (who == 3)
				SC ^= j1 ^ ((SC & j1) << 1);		//00->01->10->11->00
			else if (who == 1)//it's impossible to change on simple else because who could be equal zero
				SC ^= (0x3ffffff >> (26 - j2)) ^ ((SC & j1) << 1);

			ccode = st[j2 >> 1][SC];

			sure += (ccode & 15);
			s0 = ((ccode >> 8) & 15);
			s2 = ((ccode >> 12) & 15);
			if (ccode & 0x10) {
				b0 = 1;
				if (s2 < min02)
					min02 = s2;
			}
			if (ccode & 0x20) {
				b2 = 1;
				m0 += s0;
				if (s0 < min02)
					min02 = s0;
			}
			sure0 += s0;
			sure2 += s2;

		}		//for(j)
		if (b2) {
			if (b0)
				sure += sure0 + sure2 - min02;
			else
				sure += sure0 + sure2 - m0;
		}
		else
			sure += sure0;

		if (sure > depth)
			sure = depth;

		if ((sure << 1) > alpha + depth) {
			return alpha + 2;
		}
	}		//if(trump==NT)

	//END_SURE_TRICKS*/
#endif

	phash = &(hashtable[(KeyZob ^ (who << SHIFT)) & AND_KEY].i[0]);
	for (j1 = 0; j1 < int(HASH_ELEMENTS); ++j1, phash++) {
		if (phash->code[0] == code[0] && phash->code[1] == code[1]
				&& phash->code[2] == code[2] && phash->_code == _highcode) {
			if (phash->flag == HASH_ALPHA) {
				if (phash->value <= alpha) {
					return alpha;
				}
			}
			else {
				if (phash->value > alpha) {
					return alpha + 2;
				}
			}
		}
	}		//*/

#ifdef QUICK_TRICK
	//@@quick trick
	//BEGIN quick trick
	if (trump != NT) {
		if (alpha == -depth)	//one quick trick for leading palyer
				{
			//pointers for highest trumps on players who1,who2,who3
			ListItem* Who1Trump = NULL, *Who2Trump = NULL, *Who3Trump = NULL;

			_suit = trump;	//same with trump==4
			CurPtr = q[_suit].prev;
			if (CurPtr != &d[_suit]) {
				if (CurPtr->who == who || CurPtr->who == who2) {
					return alpha + 2;
				}
				else {
					do {
						if (CurPtr->who == who1 && Who1Trump == NULL)
							Who1Trump = CurPtr;
						else if (CurPtr->who == who2 && Who2Trump == NULL)
							Who2Trump = CurPtr;
						else if (CurPtr->who == who3 && Who3Trump == NULL)
							Who3Trump = CurPtr;
					} while ((CurPtr = CurPtr->prev) != &d[_suit]);
				}	//else
			}

			bool WhoHasSuit, Who1HasSuit, Who2HasSuit, Who3HasSuit;
			//cheking for all suits except trump suit
			for (_suit = 0; _suit < 4; ++_suit) {
				if (_suit == trump)
					continue;
				WhoHasSuit = Who1HasSuit = Who2HasSuit = Who3HasSuit = false;

				CurPtr = q[_suit].prev;
				if (CurPtr != &d[_suit]) {
					if (CurPtr->who == who) {	//highest card in a suit of player 'who'

						while ((CurPtr = CurPtr->prev) != &d[_suit]) {
							if (CurPtr->who == who1)
								Who1HasSuit = true;
							else if (CurPtr->who == who2)
								Who2HasSuit = true;
							else if (CurPtr->who == who3)
								Who3HasSuit = true;
						}
						if (Who1HasSuit) {
							if (Who3HasSuit || Who3Trump == NULL)
								return alpha + 2;
							if (!Who2HasSuit && Who2Trump != NULL
									&& Who2Trump->card > Who3Trump->card)
								return alpha + 2;
						}
						else if (Who3HasSuit) {
							//if(Who3HasSuit)return alpha+2;
							if (Who1Trump == NULL)
								return alpha + 2;
							if (!Who2HasSuit && Who2Trump != NULL
									&& Who2Trump->card > Who1Trump->card)
								return alpha + 2;
						}
						else { //who1 & who3 have no suit
							if (Who1Trump == NULL) {
								if (Who3Trump == NULL)
									return alpha + 2;
								if (!Who2HasSuit && Who2Trump != NULL
										&& Who2Trump->card > Who3Trump->card)
									return alpha + 2;
							}
							else if (Who3Trump == NULL) { //who1 has trump
								if (!Who2HasSuit && Who2Trump != NULL
										&& Who2Trump->card > Who1Trump->card)
									return alpha + 2;
							}
							else { //who1 & who3 have trump
								if (!Who2HasSuit && Who2Trump != NULL
										&& Who2Trump->card > Who1Trump->card
										&& Who2Trump->card > Who3Trump->card)
									return alpha + 2;
							}
						}

					} //if(CurPtr->who==who) highest card in a suit of player 'who'
					else if (CurPtr->who == who1) {
						while ((CurPtr = CurPtr->prev) != &d[_suit]) {
							if (CurPtr->who == who)
								WhoHasSuit = true;
							else if (CurPtr->who == who2)
								Who2HasSuit = true;
							else if (CurPtr->who == who3)
								Who3HasSuit = true;
						}
						//to take a trick 'who' should have a suit, and 'who2' hasnt it and hasnt trumps
						if (WhoHasSuit && !Who2HasSuit && Who2Trump != NULL
								&& (Who3Trump == NULL || Who2Trump->card > Who3Trump->card))
							return alpha + 2;
					}
					else if (CurPtr->who == who3) {
						while ((CurPtr = CurPtr->prev) != &d[_suit]) {
							if (CurPtr->who == who)
								WhoHasSuit = true;
							else if (CurPtr->who == who1)
								Who1HasSuit = true;
							else if (CurPtr->who == who2)
								Who2HasSuit = true;
						}
						//to take a trick 'who' should have a suit, and 'who2' hasnt it and hasnt trumps
						if (WhoHasSuit && !Who2HasSuit && Who2Trump != NULL
								&& (Who1Trump == NULL || Who2Trump->card > Who1Trump->card))
							return alpha + 2;
					}
					else if (CurPtr->who == who2) {
						while ((CurPtr = CurPtr->prev) != &d[_suit]) {
							if (CurPtr->who == who) {
								if (Who1Trump == NULL && Who3Trump == NULL)
									return alpha + 2;
								break;
							}
						}
					} //else (who2)
				}
			} //for
		} //if
		else if (alpha + 2 == depth) //one quick trick for opponents
				{
			_suit = trump; //same with trump==4
			CurPtr = q[_suit].prev;
			if (CurPtr != &d[_suit]) {
				if (CurPtr->who == who1 || CurPtr->who == who3) {
					return alpha;
				}
			}
		} //if
		else if (alpha == -depth + 2) //two quick trick on leading side
				{
			_suit = trump; //same with trump==4
			CurPtr = q[_suit].prev;
			if (CurPtr != &d[_suit]) {
				if (CurPtr->who == who) {
					CurPtr = CurPtr->prev;
					if (CurPtr != &d[_suit]) {
						if (CurPtr->who == who)
							return alpha + 2;
						if (CurPtr->who == who2) { //try to find covering
							while ((CurPtr = CurPtr->prev) != &d[_suit])
								if (CurPtr->who == who || CurPtr->who == who2)
									return alpha + 2;
						}
					}
				}
				else if (CurPtr->who == who2) {
					CurPtr = CurPtr->prev;
					if (CurPtr != &d[_suit]) {
						if (CurPtr->who == who2)
							return alpha + 2;
						if (CurPtr->who == who) { //try to find covering
							while ((CurPtr = CurPtr->prev) != &d[_suit])
								if (CurPtr->who == who || CurPtr->who == who2)
									return alpha + 2;
						}
					}
				}
			}
		}
		else if (alpha + 4 == depth) //two fast tricks on opponent side
				{
			_suit = trump; //same with trump==4
			CurPtr = q[_suit].prev;
			if (CurPtr != &d[_suit]) {
				if (CurPtr->who == who1) {
					CurPtr = CurPtr->prev;
					if (CurPtr->who == who1)
						return alpha;
					if (CurPtr->who == who3) { //find covering
						while ((CurPtr = CurPtr->prev) != &d[_suit])
							if (CurPtr->who == who1 || CurPtr->who == who3)
								return alpha;
					}
				}
				else if (CurPtr->who == who3) {
					CurPtr = CurPtr->prev;
					if (CurPtr->who == who3)
						return alpha;
					if (CurPtr->who == who1) { ////find covering
						while ((CurPtr = CurPtr->prev) != &d[_suit])
							if (CurPtr->who == who1 || CurPtr->who == who3)
								return alpha;
					}
				}
			}
		} //else if
	} //if(trump!=NT)
		//END quick trick
#endif

	return 100;

}
int BridgePosition::g(int alpha) {

#ifdef BRIDGE_NODE_COUNT
	nodeCount++;
#endif
	ListItem*CurPtr;
	unsigned j1, j2, j3, *ptr = code;
	bool w;

	////begin code generation
	for (j1 = 0; j1 < 3; ++j1, ptr++) {
		*ptr = _CodeArray[SSCODE[j1]];
	}
	_code = _CodeArray[SSCODE[3]];
	////end   code generation

//#define SHOW_MOVES_ORDER

#ifdef SHOW_MOVES_ORDER
	std::vector<ListItem*> v;
#endif


	int alpha1, alpha2, alpha3, alpha4;
	ListItem*Up[4];

	for (_suit = 0; _suit < 4; ++_suit) {
		CurPtr = q[_suit].prev;
		while (CurPtr != &d[_suit]) {
			if (CurPtr->who == who) {
#ifdef SHOW_MOVES_ORDER
				v.push_back(CurPtr);
#endif
				_card = CurPtr->card;

				find_suitable_cards();
				Remove(CurPtr)
#include "binc2.h"
				Restore(CurPtr)
				if (alpha1 > alpha) {
					RecordHash(alpha1, HASH_BETA, who);
					return alpha1;
				}
				do {
					CurPtr = CurPtr->prev;
				} while (CurPtr->who == who);
				break;
			}	//if(CurPtr->who==who)
			else
				CurPtr = CurPtr->prev;
		}	//while
		Up[_suit] = CurPtr->prev;
	}	//for(_suit)

	for (_suit = 0; _suit < 4; ++_suit) {
		w = true;
		CurPtr = Up[_suit];
		while (CurPtr != &d[_suit]) {
			if (CurPtr->who == who) {
#ifdef SHOW_MOVES_ORDER
				v.push_back(CurPtr);
#endif
				_card = CurPtr->card;
				if (w) {
					w = false;
					find_suitable_cards();
				}
				Remove(CurPtr)
#include "binc2.h"
				Restore(CurPtr)
				if (alpha1 > alpha) {
					RecordHash(alpha1, HASH_BETA, who);
					return alpha1;
				}
				do {
					CurPtr = CurPtr->prev;
				} while (CurPtr->who == who);
			}	//if(CurPtr->who==who)
			else
				CurPtr = CurPtr->prev;
		}	//while
	}	//for(_suit)

#ifdef SHOW_MOVES_ORDER
	if(v.size()>7 && trump!=0){
		std::string s=format("\ntrump=%d",int(trump));
		for(auto&w:v){
			s+=' ';
			s+=RANK[12-w->card];
			s+=SUITS_CHAR[w->suit];
		}
		printl(s)
		exit(0);
	}
#endif

	RecordHash(alpha, HASH_ALPHA, who);
	return alpha;
}

int BridgePosition::fg(int alpha) {
	const int beta = alpha + 2;
	ListItem*CurPtr;
	for (_suit = 0; _suit < 4; ++_suit) {
		CurPtr = &q[_suit];
		while ((CurPtr = CurPtr->prev) != &d[_suit]) {
			if (CurPtr->who == who) {
				bc = CurPtr->card;
				bs = _suit;
			}
		}	//while
	}
	if (next == NULL)
		return lastTrickValue();

	unsigned j1, j2, j3;
	int alpha1, alpha2, alpha3, alpha4;
	bool w, t;
	for (_suit = 0; _suit < 4; ++_suit) {
		w = true;
		t = true;
		CurPtr = q[_suit].prev;	//it's impossible to do this variable as global
		if (CurPtr != &d[_suit])
			do {
				if (CurPtr->who == who) {
					_card = CurPtr->card;
					if (t) {
						if (w) {
							w = false;
							find_suitable_cards();
						}
						Remove(CurPtr)
#define bestmove1
#include "binc2.h"
#undef bestmove1
						Restore(CurPtr)
						if (alpha1 > alpha) {
							alpha = alpha1;
							bc = _card;
							bs = _suit;
							if (alpha >= beta)
								return beta;

						}
						t = false;
					}	//if(t)
				}	//if(CurPtr->who==who)
				else
					t = true;
			} while ((CurPtr = CurPtr->prev) != &d[_suit]);
	}
	return alpha;
}
int BridgePosition::fg1(int _alpha, BridgePosition::ListItem*_P0) {
	const int _beta = _alpha + 2;

	_suit = _P0->suit;
	_card = _P0->card;
	Restore1(_P0);
	find_suitable_cards();
	Remove1(_P0);

	bs = suitable_t1[startIndex]->suit;
	bc = suitable_t1[startIndex]->card;
	if (next == NULL)
		return -lastTrickValue();

	int alpha = -_beta;
	int alpha1;	//=beta dont need to define
	int alpha3, alpha4;
	unsigned j1, j2, j3;
	int alpha2;	//value
#define bestmove2
#include "binc2.h"
#undef bestmove2
	return -alpha1;
}

int BridgePosition::fg2(int _alpha, BridgePosition::ListItem* _P0,
		BridgePosition::ListItem* _P1) {
	const int _beta = _alpha + 2;

	_suit = _P0->suit;
	_card = _P0->card;
	Restore1(_P0);
	Restore1(_P1);
	find_suitable_cards();
	Remove1(_P0);
	Remove1(_P1);

	bs = suitable_t2[0]->suit;
	bc = suitable_t2[0]->card;
	if (next == NULL)
		return lastTrickValue();

	p1 = _P1;

	int alpha1 = _beta;
	int alpha2;	//=alpha
	int alpha3;	//value
	int alpha4;
	unsigned j2, j3;
#define bestmove3
#define alpha _alpha
#include "binc3.h"
#undef alpha
#undef bestmove3
	return alpha2;

}

int BridgePosition::fg3(int _alpha, BridgePosition::ListItem*_P0,
		BridgePosition::ListItem*_P1, BridgePosition::ListItem*_P2) {
	const int _beta = _alpha + 2;
	//unsigned*ptr;
	_suit = _P0->suit;
	_card = _P0->card;
	Restore1(_P0);
	Restore1(_P1);
	Restore1(_P2);
	find_suitable_cards();
	Remove1(_P0);
	Remove1(_P1);
	Remove1(_P2);

	bs = suitable_t3[0]->suit;
	bc = suitable_t3[0]->card;
	if (next == NULL)
		return -lastTrickValue();

	p1 = _P1;
	p2 = _P2;

	int alpha1 = -_alpha;
	int alpha2 = -_beta;
	int alpha3;	//no essential
	int alpha4;
	unsigned j3;
#define bestmove4
#include "binc4.h"
#undef bestmove4

	return -alpha3;
}
/////////////////////////////
#undef XOR
#undef Remove
#undef Restore
#undef RecordHash
#undef KeyZob
/////////////////////////////

//SUITABLE CARDS
//#define inverse_trump
//#define inverse_after
#define reorder
//#define reorder_first

void BridgePosition::find_suitable_cards() {
	counter_suitable_cards1 = counter_suitable_cards2 = counter_suitable_cards3 = 0;
	unsigned char ss = 0;
#ifdef reorder
	int j1 = 0, j2 = 0, j3 = 0;
#endif
	int j;
	ListItem*CurPtr = q[_suit].prev;
	while (CurPtr != &d[_suit]) {
		if (CurPtr->who == who1) {
			suitable_t1[counter_suitable_cards1++] = CurPtr;
			do {
				CurPtr = CurPtr->prev;
			} while (CurPtr->who == who1);
			ss |= 1;
		}
		else if (CurPtr->who == who2) {
			suitable_t2[counter_suitable_cards2++] = CurPtr;
			do {
				CurPtr = CurPtr->prev;
			} while (CurPtr->who == who2);
			ss |= 2;
		}
		else if (CurPtr->who == who3) {
			suitable_t3[counter_suitable_cards3++] = CurPtr;
			do {
				CurPtr = CurPtr->prev;
			} while (CurPtr->who == who3);
			ss |= 4;
		}
		else {
			do {
				CurPtr = CurPtr->prev;
			} while (CurPtr->who == who);
		}
	}	//while


//	const int m=3;
//	if(counter_suitable_cards1 > m || counter_suitable_cards2 > m || counter_suitable_cards3 > m){
//		printSuitableCards();
//		printinfo
//		exit(0);
//	}

#ifdef reorder_first
	//swap 2nd and last card
#define M(I)  \
	  if(counter_suitable_cards##I > 2)\
    {\
		  CurPtr=suitable_t##I[counter_suitable_cards##I-1];\
		  suitable_t##I[counter_suitable_cards##I-1]=suitable_t##I[1];\
		  suitable_t##I[1]=CurPtr;\
	  }
#define MM {M(1)M(2)M(3)return;}
#else
#define M
#define MM	return;
#endif

	if (ss == 7)
		MM

	if (ss == 3) {
#define ctr counter_suitable_cards3
#define sui suitable_t3
#define _who who3
#include "bsc.h"
#undef _who
#undef ctr
#undef sui
		MM
	}
	if (ss == 5) {
#define ctr counter_suitable_cards2
#define sui suitable_t2
#define _who who2
#include "bsc.h"
#undef _who
#undef ctr
#undef sui
		MM
	}
	if (ss == 6) {
#define ctr counter_suitable_cards1
#define sui suitable_t1
#define _who who1
#include "bsc.h"
#undef _who
#undef ctr
#undef sui
		MM
	}
	if (ss == 1) {
#define ctr1 counter_suitable_cards2
#define ctr2 counter_suitable_cards3
#define sui1 suitable_t2
#define sui2 suitable_t3
#define _who1 who2
#define _who2 who3
#include "bsc1.h"
#undef _who1
#undef _who2
#undef ctr1
#undef ctr2
#undef sui1
#undef sui2
		MM
	}
	if (ss == 2) {
#define ctr1 counter_suitable_cards1
#define ctr2 counter_suitable_cards3
#define sui1 suitable_t1
#define sui2 suitable_t3
#define _who1 who1
#define _who2 who3
#include "bsc1.h"
#undef _who1
#undef _who2
#undef ctr1
#undef ctr2
#undef sui1
#undef sui2
		MM
	}
	if (ss == 4) {
#define ctr1 counter_suitable_cards1
#define ctr2 counter_suitable_cards2
#define sui1 suitable_t1
#define sui2 suitable_t2
#define _who1 who1
#define _who2 who2
#include "bsc1.h"
#undef _who1
#undef _who2
#undef ctr1
#undef ctr2
#undef sui1
#undef sui2
		MM
	}
	//trumps && all cards
	if (trump != NT) {
#ifdef inverse_trump
		CurPtr=q[trump].prev;
		while(CurPtr!=&d[trump]) {
#else
		CurPtr = d[int(trump)].next;
		while (CurPtr != &q[int(trump)]) {
#endif
			if (CurPtr->who == who1) {
#ifdef reorder
				j1 = 1;
#endif
				suitable_t1[counter_suitable_cards1++] = CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->prev;}while(CurPtr->who==who1);
#else
				do {
					CurPtr = CurPtr->next;
				} while (CurPtr->who == who1);
#endif
			}
			else if (CurPtr->who == who2) {
#ifdef reorder
				j2 = 1;
#endif
				suitable_t2[counter_suitable_cards2++] = CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->prev;}while(CurPtr->who==who2);
#else
				do {
					CurPtr = CurPtr->next;
				} while (CurPtr->who == who2);
#endif
			}
			else if (CurPtr->who == who3) {
#ifdef reorder
				j3 = 1;
#endif
				suitable_t3[counter_suitable_cards3++] = CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->prev;}while(CurPtr->who==who3);
#else
				do {
					CurPtr = CurPtr->next;
				} while (CurPtr->who == who3);
#endif
			}
			else {
#ifdef inverse_trump
				do {CurPtr=CurPtr->prev;}while(CurPtr->who==who);
#else
				do {
					CurPtr = CurPtr->next;
				} while (CurPtr->who == who);
#endif
			}
		}	//while()
	}	//end if(trump!=NT)
	for (j = 0; j < 4; ++j)
		if (j != trump && j != _suit) {
#ifdef reorder
			ss = 7;
#endif

#ifdef inverse_after
			CurPtr=q[j].prev;
			while(CurPtr!=&d[j]) {
#else
			CurPtr = d[j].next;
			while (CurPtr != &q[j]) {
#endif
				if (CurPtr->who == who1) {
					suitable_t1[counter_suitable_cards1++] = CurPtr;
#ifdef reorder
					if (ss & 1) {
						suitable_t1[counter_suitable_cards1 - 1] = suitable_t1[j1];
						suitable_t1[j1] = CurPtr;
						j1++;
						ss ^= 1;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->prev;}while(CurPtr->who==who1);
#else
					do {
						CurPtr = CurPtr->next;
					} while (CurPtr->who == who1);
#endif
				}
				else if (CurPtr->who == who2) {
					suitable_t2[counter_suitable_cards2++] = CurPtr;
#ifdef reorder
					if (ss & 2) {
						suitable_t2[counter_suitable_cards2 - 1] = suitable_t2[j2];
						suitable_t2[j2] = CurPtr;
						j2++;
						ss ^= 2;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->prev;}while(CurPtr->who==who2);
#else
					do {
						CurPtr = CurPtr->next;
					} while (CurPtr->who == who2);
#endif
				}
				else if (CurPtr->who == who3) {
					suitable_t3[counter_suitable_cards3++] = CurPtr;
#ifdef reorder
					if (ss & 4) {
						suitable_t3[counter_suitable_cards3 - 1] = suitable_t3[j3];
						suitable_t3[j3] = CurPtr;
						j3++;
						ss ^= 4;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->prev;}while(CurPtr->who==who3);
#else
					do {
						CurPtr = CurPtr->next;
					} while (CurPtr->who == who3);
#endif
				}
				else {
#ifdef inverse_after
					do {CurPtr=CurPtr->prev;}while(CurPtr->who==who);
#else
					do {
						CurPtr = CurPtr->next;
					} while (CurPtr->who == who);
#endif
				}
			}	//while
		}	//for

//		const int m=8;
//		if(trump==NT){
//			if(counter_suitable_cards1 > m || counter_suitable_cards2 > m || counter_suitable_cards3 > m){
//				printSuitableCards();
//				printinfo
//				exit(0);
//			}
//		}

	MM
}
#undef M
#undef MM
//using in fg.... function
int BridgePosition::lastTrickValue() {
#ifdef BRIDGE_NODE_COUNT
	nodeCount++;
#endif
	ListItem*CurPtr;

	for (int curs = 0; curs < 4; ++curs) {
		CurPtr = &q[curs];
		while ((CurPtr = CurPtr->prev) != &d[curs]) {
			if (CurPtr->who == who)
				_suitcard = CurPtr->suitcard;
			else if (CurPtr->who == who1)
				p1 = CurPtr;
			else if (CurPtr->who == who2)
				p2 = CurPtr;
			else
				p3 = CurPtr;
		}
	}
	if ((compare(_suitcard,p1->suitcard)&&
	compare(_suitcard,p2->suitcard) &&
	compare(_suitcard,p3->suitcard)) ||
	(!compare(_suitcard,p2->suitcard) &&
	compare(p2->suitcard,p1->suitcard) &&
	compare(p2->suitcard,p3->suitcard)))return +1;
	return -1;
}

bool BridgePosition::hasSuit(int suit, int player, const int*cid) {
	if (suit == 4)
		return false;
	for (int i = suit * 13; i < (suit + 1) * 13; i++)
		if (cid[i] == player + 1)
			return true;

	return false;
}

bool BridgePosition::solve(const int ptr[52], char _trump, int firstmove,
		bool trumpChanged) {
	BridgePosition::trumpChanged = trumpChanged;
	guess = 0;
	maximize = 0;
	trump = _trump;
	BridgePosition::firstmove = firstmove;

	int i, j;
	for (i = j = 0; i < 52; i++) {
		if (ptr[i] >= 5 && ptr[i] <= 8) {
			j++;
		}
	}
	BridgePosition::nextmove = (firstmove + j) % 4;

	return BridgePosition::solveposition(ptr, _trump);
}

void BridgePosition::printSuitableCards(){
	unsigned i,j;
	std::string s=format("\ntrump=%d",int(trump));

	unsigned c[]={counter_suitable_cards1,counter_suitable_cards2,counter_suitable_cards3};
	ListItem** p[]={suitable_t1,suitable_t2,suitable_t3};

	for(j=0;j<3;j++){
		s+="\n";
		for(i=0;i<c[j];i++){
			auto w=p[j][i];
			s+=RANK[12-w->card];
			s+=SUITS_CHAR[w->suit];
			s+=' ';
		}
	}

	printl(s)
}
