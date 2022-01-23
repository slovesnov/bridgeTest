/*
 * pcommon.h
 *
 *  Created on: 20.01.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef PCOMMON_H_
#define PCOMMON_H_

#include "Deal.h"

enum {
	SPADES,HEARTS,DIAMONDS,CLUBS
};

const CARD_INDEX PREFERANS_PLAYER[] = {
		CARD_INDEX_WEST,
		CARD_INDEX_NORTH,
		CARD_INDEX_EAST };

Deal preferansDeal[]= {
	//"north east west" cards
	Deal(" A98.AT98..T87 QT7.KQJ.A.KQJ KJ.7.KQJT9.A9 ",SPADES,CARD_INDEX_WEST,CARD_INDEX_EAST)//deal1 e=10
	, Deal(" J9.97.8.87 87.KQ.KQ.J QT.A8.A97. ",SPADES,CARD_INDEX_NORTH,CARD_INDEX_EAST)//deal2 e=7
	, Deal(" 87.KQ.KQ.J QT.A8.A97. J9.97.8.87 ",SPADES,CARD_INDEX_WEST,CARD_INDEX_NORTH)//deal2' e=7
	, Deal(" QJT.J9.A97.KJ K9.Q87.QT8.QT A.AKT.KJ.A987 ",CLUBS,CARD_INDEX_WEST,CARD_INDEX_WEST)//deal3 e=7
	, Deal("AJ98.87.87.87 .KT9.AQT.KQT9 KQT7.AQJ.KJ.A",SPADES,CARD_INDEX_WEST,CARD_INDEX_WEST)//deal4 e=4 time=53.6
	, Deal(" T987.98.987.8 AK.AKQT.J.QJT QJ.J7.AKQT.97 ",NT,CARD_INDEX_WEST,CARD_INDEX_NORTH,true)//deal5
	, Deal(" 8.T987.987.98 AT9.KQ.K.AQJT J7.AJ.AQJT.K7 ",NT,CARD_INDEX_WEST,CARD_INDEX_NORTH,true)//deal 6
	, Deal("A8.AJ7.AJ8.KT QT.KT8.KT7.Q8 KJ7.Q9.Q9.AJ7",NT,CARD_INDEX_WEST,CARD_INDEX_NORTH)//deal7 e=7 time=240s
};
const int preferansDealSize=SIZEI(preferansDeal);
const int preferansDealE[]= {10,7,7,7,4, 8,8, 7};
static_assert(SIZEI(preferansDealE)==preferansDealSize);


#endif /* PCOMMON_H_ */
