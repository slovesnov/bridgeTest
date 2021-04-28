/*
 * pe2.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

int i, v, suit2, card2, t, r2;
#ifdef PREFERANS_NODE_COUNT
m_nodes++;
#endif

for (i = 0; i < c2.length; i += 2) {
	r2 = card2 = c2[i];
	suit2 = c2[i + 1];

	if (suit2 == suit) {
		if (r2 > card) {
			r2--;
		}
	}
	if (suit2 == suit1) {
		if (r2 > r1) {
			r2--;
		}
	}

	removeCard(suit2, r2);

	if (compare(suit, card, suit1, card1)) {
		t = compare(suit, card, suit2, card2) ? 0 : 2;
	}
	else {
		t = compare(suit1, card1, suit2, card2) ? 1 : 2;
	}

	m_depth--;
	if ((t == 0 && w[1] == 0) || (t == 1 && w[0] == 0) || t == 2) {
#ifdef MIZER
		v = -1;
		v += eMizer(w[t], a - v, b - v);
#else
		v = 1;
		v += e(w[t], a - v, b - v);
#endif

	}
	else {
#ifdef MIZER
		v = 1;
		v -= eMizer(w[t], -b + v, -a + v);
#else
		v = -1;
		v -= e(w[t], -b + v, -a + v);
#endif

	}
	m_depth++;

	restoreCard(suit2, r2, w[2]);
	if (v > a) {
#ifdef STOREBEST
		m_best = suit2 * 13 + card2;
#endif
		if ((a = v) >= b) {
			return b;
		}
	}
}
return a;
