/*
 * DealData.h
 *
 *  Created on: 15.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DEALDATA_H_
#define DEALDATA_H_

#include <string>

#include "BridgeCommon.h"

void run(int nodes, int problem, bool old,int*result);

class DealData {
public:
	std::string deal;
	int trump;//0..3,NT - no trump, NT+1 - misere and no trump
	int first;
	std::string comment;
	bool longProblem;

	bool misere()const{
		return trump==NT+1;
	}

	bool nt()const{
		return trump>=NT;
	}

	std::string cppString()const;

};

/* Note problem1 from preferansRu is not used because each player
 * has only seven cards. 4 deals from solvealldeals file, and 6 from preferansRu
 */
const DealData dealData[]={
		//comments from library problems/solvealldeals.pts
	{ "T987.T987.*98.*Q#J", MISERE, 0, "solvealldeals1-1", false },
	{ "T987.T987.#98.*Q*J", MISERE, 0, "solvealldeals1-2", false },
	{ "#A*8*7.AKT.KJ.A987", 3,      0, "solvealldeals2", false },
	{ "AQJ*T8*7.KT8.KJ.9", 0,       2, "solvealldeals3", true },

		//comments from library problems/preferansRu.bts
	{ "QT7.KQJ.A*8*7.KQJ", 0,      2, "preferansRu1",false },
	{ "A*8*7.AKT.KJ.A987", 3,      2, "preferansRu3",false },
	{ "KQT7.AQJ.KJ*9.A*J", 0,      2, "preferansRu4",true },
	{ "T987.98.987.*A*K8", MISERE, 2, "preferansRu5",true },
	{ "*K*Q8.T987.987.98", MISERE, 2, "preferansRu6",true },
	{ "A*98.AJ7.AJ8.KT*9", NT,     2, "preferansRu7",true }
};

const int RESULT_SIZE = 11;

/* need to define here
 * SOLVE_TYPE 0 184 756 positions
 * SOLVE_TYPE 1 20 000 positions
 */
#define SOLVE_TYPE 1

#if SOLVE_TYPE==0
const int PREFERANS_SOLVE_ALL_DEALS_POSITIONS=184756;
const int results[][RESULT_SIZE] = {
		{8544, 147660, 1340, 9050, 15156, 2876, 130, 0, 0, 0, 0},
		{103936, 13832, 612, 9362, 35612, 20754, 648, 0, 0, 0, 0},
		{0, 0, 0, 0, 1016, 19625, 104962, 58993, 160, 0, 0},
		{0, 0, 0, 3015, 134666, 46917, 158, 0, 0, 0, 0},
		{441, 4547, 18204, 78510, 78762, 4292, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1376, 29432, 100086, 53802, 60, 0, 0},
		{0, 0, 0, 0, 1226, 75131, 104670, 3699, 30, 0, 0},
		{80407, 977, 23345, 49263, 22306, 8207, 251, 0, 0, 0, 0},
		{80407, 977, 23345, 49263, 22306, 8207, 251, 0, 0, 0, 0},
		{0, 0, 0, 102170, 76768, 5816, 2, 0, 0, 0, 0}
};
#else
const int PREFERANS_SOLVE_ALL_DEALS_POSITIONS = 20000;
const int results[][RESULT_SIZE] = {
		{906, 13958, 366, 2054, 2384, 325, 7, 0, 0, 0, 0},
		{7978, 2338, 233, 2062, 5021, 2308, 60, 0, 0, 0, 0},
		{0, 0, 0, 0, 226, 1643, 11300, 6799, 32, 0, 0},
		{0, 0, 0, 0, 12740, 7222, 38, 0, 0, 0, 0},
		{0, 229, 828, 9058, 9807, 78, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 523, 4578, 9569, 5318, 12, 0, 0},
		{0, 0, 0, 0, 398, 14262, 5173, 167, 0, 0, 0},
		{3345, 454, 4914, 7402, 3117, 742, 26, 0, 0, 0, 0},
		{6259, 241, 2728, 5349, 3523, 1774, 126, 0, 0, 0, 0},
		{0, 0, 0, 10895, 8338, 767, 0, 0, 0, 0, 0}
};

#endif

#endif /* DEALDATA_H_ */
