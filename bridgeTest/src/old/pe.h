/*
 * pe.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

int i, j, t, v, suit, card, c;
int w[] = { ww, (ww + 1) % 3, (ww + 2) % 3 };

#ifdef PREFERANS_NODE_COUNT
m_nodes++;
#endif

if (m_depth == 1) {
	int card1, suit1, card2, suit2;

	for (i = 0; i < 4; i++) {
		for (j = 0, c = m_code[i]; c != 3; c >>= 2, j++) {
			t = c & 3;
			if (t == w[0]) {
				card = j;
				suit = i;
			}
			else if (t == w[1]) {
				card1 = j;
				suit1 = i;
			}
			else {
				card2 = j;
				suit2 = i;
			}
		}
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	if (compare(suit, card, suit1, card1)) {
		t = compare(suit, card, suit2, card2) ? 0 : 2;
	}
	else {
		t = compare(suit1, card1, suit2, card2) ? 1 : 2;
	}
#pragma GCC diagnostic pop

#ifdef MIZER
	v = t == 0 || (t == 1 && w[2] == 0) || (t == 2 && w[1] == 0) ? -1 : 1;
#else
	v = t == 0 || (t == 1 && w[2] == 0) || (t == 2 && w[1] == 0) ? 1 : -1;
#endif
	return v;
}

if (a >= m_depth) {
	return a;
}
if (b <= -m_depth) {
	return b;
}
if (a < -m_depth) {
	a = -m_depth;
}
if (b > m_depth) {
	b = m_depth;
}

//probeHash
#ifndef STOREBEST
Hash& h = m_hashTable[hashIndex(w[0])];
if (h.f != HASH_INVALID) {
	//int16_t(m_code[i]) clear two high bits if cards in suit=8
	for (i = 0; i < 3 && h.code[i] == int16_t(m_code[i]); i++)
	;
	if (i == 3) {
		if (h.f == HASH_EXACT) {
			return h.v;
		}
		if (h.f == HASH_ALPHA && h.v <= a) {
			return a;
		}
		if (h.f == HASH_BETA && h.v >= b) {
			return b;
		}
	}
}
#endif
int8_t f = HASH_ALPHA;

SC ca;
SC p[4];
for (i = 0; i < 4; i++) {
	suitableCards(i, w[0], p[i]);
}
for (auto const& t : p) {
	if (t.length > 0) {
		ca.push(t[0], t[1]); //highest card in suit
	}
}
for (auto const& t : p) {
	for (j = 2; j < t.length; j++) { //all except first one
		ca.push(t[j]);
	}
}

#ifdef STOREBEST
m_best = ca[1] * 13 + ca[0];
#endif

SC c1, c2;
for (i = 0; i < ca.length; i += 2, c1.length = c2.length = 0) {
	card = ca[i];
	suit = ca[i + 1];

	suitableCards2P(suit, w, c1, c2);

	removeCard(suit, card);

	if (w[2] == 0) {
#ifdef MIZER
		v = e1Mizer(w, a, b, suit, card, c1, c2);
#else
		v = e1(w, a, b, suit, card, c1, c2);
#endif
	}
	else {
#ifdef MIZER
		v = -e1Mizer(w, -b, -a, suit, card, c1, c2);
#else
		v = -e1(w, -b, -a, suit, card, c1, c2);
#endif
	}

	restoreCard(suit, card, w[0]);
	if (v > a) {
#ifdef STOREBEST
		m_best = suit * 13 + card;
#endif
		f = HASH_EXACT;
		if ((a = v) >= b) {
			recordHash(b, HASH_BETA, w[0]);
			return b;
		}
	}
}
recordHash(a, f, w[0]);
return a;
