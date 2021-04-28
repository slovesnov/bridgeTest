alpha1=alpha+2;

#ifdef BRIDGE_NODE_COUNT
nodeCount++;
#endif

for(j1=
#ifdef bestmove2
		startIndex
#else
		0
#endif
		;j1<counter_suitable_cards1;++j1) {

	p1=suitable_t1[j1];

	Remove(p1)
#include "binc3.h"
	Restore(p1)
	if(alpha2<alpha1) {
		alpha1=alpha2;
#ifdef bestmove2
		bs=p1->suit;
		bc=p1->card;
		startIndex=j1;
#endif
		break;
	}

} // for
