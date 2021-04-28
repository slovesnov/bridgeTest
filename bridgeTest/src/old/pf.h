/*
 * pf.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

int fw[] = { ww, (ww + 1) % 3, (ww + 2) % 3 };
int v;
SC c1, c2;

if (m_cards == 1) {
		//prevents gcc warnings init m[] here we don't need max speed
		int m[] = { 0, 0, 0 };
		int i, j, t, c;

		//cards on table
		int n = suit == -1 ? 0 : (suit1 == -1 ? 1 : 2);

		for (i = 0; i < 4; i++) {
			for (j = 0, c = m_code[i]; c != 3; c >>= 2, j++) {
				t = c & 3;
				for (v = 0; v < 3 && t != fw[v]; v++)
					;
				assert(v < 3);
				m[v] = i * 13 + j;
				if (n == v) {
					m_best = m[v];
				}
			}
		}

#define C(x,y) compare(m[x]/13,m[x]%13,m[y]/13,m[y]%13)
		if (C(0, 1)) {
			t = C(0, 2)? 0 : 2;
		}
		else {
			t = C(1, 2)
? 1 : 2;
}
#undef C

 //taker n or neither t nor n are preferans players
if (t == n || (fw[t] != 0 && fw[n] != 0)) {
#ifdef MIZER
v = -1;
#else
v = 1;
#endif
}
else {
#ifdef MIZER
v = 1;
#else
v = -1;
#endif
}

return v;
}

if (suit1 == -1) {
suitableCards2P(suit, fw, c1, c2);
m_best = c1[1] * 13 + c1[0];
removeCard(suit, card);

#ifdef MIZER
#ifdef STOREBEST
#define F e1bMizer
#else
#define F e1Mizer
#endif
#else //MIZER
#ifdef STOREBEST
#define F e1b
#else
#define F e1
#endif
#endif //MIZER
v = F(fw, a, b, suit, card, c1, c2);
#undef F
}
else {
suitableCardsP(suit, fw[2], c2);
m_best = c2[1] * 13 + c2[0];
assert(getW(suit, card) == ww);
removeCard(suit, card);

int r1 = card1;
if (suit1 == suit) {
if (r1 > card) {
	r1--;
}
}
assert(getW(suit1, r1) == fw[1]);
removeCard(suit1, r1);

#ifdef MIZER
#ifdef STOREBEST
#define F e2bMizer
#else
#define F e2Mizer
#endif
#else //MIZER
#ifdef STOREBEST
#define F e2b
#else
#define F e2
#endif
#endif //MIZER

v = F(fw, a, b, suit, card, suit1, card1, c2, r1);

#undef F

}
return v;

