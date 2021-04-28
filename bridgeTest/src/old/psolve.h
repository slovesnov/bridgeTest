/*
 * psolve.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

int a, i, j, k, m, n, pi[CARD_INDEX_WEST + 1], fi;
CARD_INDEX l, first1;
const CARD_INDEX *p;
int suit = -1, card = -1, suit1 = -1, card1 = -1;

for (i = 0; i < 3 && preferansPlayer[i] != player; i++)
;
for (j = 0; j < 3; j++, i++) {
	pi[preferansPlayer[i % 3]] = j;
}

//Note. Inner representation player always=0
m_trump = trump;
fi = pi[first];

for (i = 0; i < 3 && preferansPlayer[i] != first; i++)
;
first1 = preferansPlayer[(i + 1) % 3];

n = 0;
for (i = 0; i < 4; i++) {
	m = 0;
	k = 0;
	p = c + i * 13;
	a = 0;
	for (j = 0; j < 8; j++) {
		l = *p++;

		if (l == CARD_INDEX_ABSENT) {
			a++;
		}
		else {
			if (l == first + CARD_INDEX_NORTH_INNER - CARD_INDEX_NORTH) {
				l = first;
				suit = i;
				card = j - a;
			}
			else if (l == first1 + CARD_INDEX_NORTH_INNER - CARD_INDEX_NORTH) {
				l = first1;
				suit1 = i;
				card1 = j - a;
			}
			m |= pi[l] << k;
			k += 2;
		}
	}
	n += k;
	m |= 3 << k;
	m_code[i] = m;
}

//cards in each hand
m_depth = m_cards = n / 6;

if (trumpChanged) {
	Hash* t = m_hashTable;
	Hash* e = m_hashTable + m_hashSize;
	for (t = m_hashTable; t != e; t++) {
		t->f = HASH_INVALID;
	}
}
#ifdef PREFERANS_NODE_COUNT
m_nodes=0;
#endif
#ifndef NDEBUG
m_best = -1;
#endif
if (suit == -1 && m_cards != 1) {
#ifdef MIZER
#ifdef STOREBEST
#define F ebMizer
#else
#define F eMizer
#endif
#else //MIZER
#ifdef STOREBEST
#define F eb
#else
#define F e
#endif
#endif //MIZER

	i = F(fi, -m_cards, m_cards);

#undef F
}
else {
#ifdef MIZER
#ifdef STOREBEST
#define F fbMizer
#else
#define F fMizer
#endif
#else //MIZER
#ifdef STOREBEST
#define F fb
#else
#define F f
#endif
#endif //MIZER

	i = F(fi, -m_cards, m_cards, suit, card, suit1, card1);

#undef F
}
#ifdef STOREBEST
assert(m_best != -1);
#endif

#ifdef MIZER
m_e = (m_cards - i) / 2;
#else
m_e = (m_cards + i) / 2;
#endif

//cards on table
n = suit == -1 ? 0 : (suit1 == -1 ? 1 : 2);
if ((fi + n) % 3 == 0) {
	m_playerTricks = m_e;
}
else {
	m_playerTricks = m_cards - m_e;
}

#ifdef STOREBEST
//adjust best card rank
i = m_best % 13;
p = c + (m_best / 13) * 13;
a = 0;
for (j = 0; j < 8; j++, p++) {
	if (*p == CARD_INDEX_ABSENT) {
		a++;
	}
	else {
		if (i-- == 0) {
			m_best += a;
			break;
		}
	}
}
#endif
