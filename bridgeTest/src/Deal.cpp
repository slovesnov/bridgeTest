/*
 * Deal.cpp
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Deal.h"

#include <cstring>
#include <cassert>
#include <algorithm>

void Deal::parse(const char*deal, CARD_INDEX c[52]) {
	int i, j, k, l;
	const char*p;

	for (i = 0; i < 52; i++) {
		c[i] = CARD_INDEX_ABSENT;
	}
	auto d = trim(deal);
	std::transform(d.begin(), d.end(), d.begin(),
			[](unsigned char c) {return std::tolower(c);});
	auto v = split(d, " ");
	assert(v.size()==3 || v.size()==4);

	//first string is north,2nd east,3rd west
	const CARD_INDEX pl[][4] = { {
			CARD_INDEX_NORTH,
			CARD_INDEX_EAST,
			CARD_INDEX_WEST }, {
			CARD_INDEX_NORTH,
			CARD_INDEX_EAST,
			CARD_INDEX_SOUTH,
			CARD_INDEX_WEST } };

	for (i = 0; i < int(v.size()); i++) {
		k = 0;
		std::string const& s = v[i];
		for (j = 0; j < int(s.length()); j++) {
			if (s[j] == '.') {
				k++;
			}
			else {
				p = strchr(RANK, tolower(s[j]));
				assert(p);
				l = p - RANK;
				c[k * 13 + l] = pl[v.size()-3][i];
			}
		}
	}

}

