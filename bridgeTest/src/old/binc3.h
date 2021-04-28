alpha2=alpha;
#ifdef BRIDGE_NODE_COUNT
nodeCount++;
#endif
for(j2=0;j2<counter_suitable_cards2;++j2) {
	p2=suitable_t2[j2];

//additional cutoff's

#ifdef ADDCUT
#ifndef bestmove2
#ifndef bestmove3
	if(compare(_suitcard,P1->suitcard)) {
		if(j2!=0 && p2==P1->Prev && suitable_t2[j2-1]==P1->next) {
			continue;
		}
	}
	else {
		if(j2!=0 && p2==CurPtr->Prev && suitable_t2[j2-1]==CurPtr->next)continue;
	}
#endif
#endif
#endif

	Remove(p2)
#include "binc4.h"
	Restore(p2)
	if(alpha2<alpha3) {
#ifdef bestmove3
		bs=p2->suit;
		bc=p2->card;
		startIndex=j2;
#endif
		alpha2=alpha3;
		break;
	}
} // for

