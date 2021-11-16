/*
 * DealData.cpp
 *
 *  Created on: 15.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "DealData.h"

std::string DealData::cppString() const {
	//{ "T987.T987.#98.*Q*J", MISERE, 0, "solvealldeals1-1", false }
	auto wrapQuotes = [](auto s) {
	    return '"'+s+'"';
	};
	VString v = { wrapQuotes(deal),
			trump == MISERE ? "MISERE" : (trump == NT ? "NT" : toString(trump)),
			toString(first), wrapQuotes(comment), longProblem ? "true" : "false" };
	return "{"+joinV(v,", ")+"},";
}
