/*
 * Deal.h
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef DEAL_H_
#define DEAL_H_

#include "BridgeCommon.h"

class Deal {
public:
	std::string m_deal;
	int m_trump;
	bool m_misere;
	CARD_INDEX c[52], m_first, m_player; //c[suit][rank]
	Deal(const char*deal, int trump, CARD_INDEX first, CARD_INDEX player,
			bool misere = false) {
		m_deal = deal;
		m_trump = trump;
		m_first = first;
		m_player = player;
		m_misere = misere;
		parse(deal, c);
	}

	Deal(const char*deal, int trump, CARD_INDEX first) {
		m_deal = deal;
		m_trump = trump;
		m_first = first;
		m_player = CARD_INDEX_INVALID;
		m_misere = false;
		parse(deal, c);
	}

	Deal(std::initializer_list<int> ci, int trump, CARD_INDEX first) {
		assert(ci.size() == 52);
		int i=0;
		for(int a:ci){
			c[i++]=CARD_INDEX(a);
		}
		m_trump = trump;
		m_first = first;
	}

	void parse(const char*deal, CARD_INDEX c[52]);

};

using VDeal = std::vector<Deal>;


#endif /* DEAL_H_ */
