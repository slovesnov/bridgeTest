/*
 * pe1.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

int i, v, suit1, card1, r1;
#ifdef PREFERANS_NODE_COUNT
m_nodes++;
#endif

for (i = 0; i < c1.length; i += 2) {
	r1 = card1 = c1[i];
	suit1 = c1[i + 1];

	if (suit1 == suit) {
		if (r1 > card) {
			r1--;
		}
	}

	removeCard(suit1, r1);
	if (w[0] == 0) {
#ifdef MIZER
		v = e2Mizer(w, a, b, suit, card, suit1, card1, c2, r1);
#else
		v = e2(w, a, b, suit, card, suit1, card1, c2, r1);
#endif
	}
	else {
#ifdef MIZER
		v = -e2Mizer(w, -b, -a, suit, card, suit1, card1, c2, r1);
#else
		v = -e2(w, -b, -a, suit, card, suit1, card1, c2, r1);
#endif
	}

	restoreCard(suit1, r1, w[1]);
	if (v > a) {
#ifdef STOREBEST
		m_best = suit1 * 13 + card1;
#endif

		if ((a = v) >= b) {
			return b;
		}
	}
}
return a;
