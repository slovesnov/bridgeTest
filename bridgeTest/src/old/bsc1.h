#ifdef reorder
j1=j2=0; //dont remove!
#endif
if(trump!=NT) {
#ifdef inverse_trump
	CurPtr=q[trump].Prev;
	while(CurPtr!=&d[trump]) {
#else
		CurPtr=d[int(trump)].next;
		while(CurPtr!=&q[int(trump)]) {
#endif
			if(CurPtr->who==_who1) {
#ifdef reorder
				j1=1;
#endif
				sui1[ctr1++]=CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who1);
#else
				do {CurPtr=CurPtr->next;}while(CurPtr->who==_who1);
#endif
			}
			else if(CurPtr->who==_who2) {
#ifdef reorder
				j2=1;
#endif
				sui2[ctr2++]=CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who2);
#else
				do {CurPtr=CurPtr->next;}while(CurPtr->who==_who2);
#endif
			}
			else {
#ifdef inverse_trump
				CurPtr=CurPtr->Prev;
#else
				CurPtr=CurPtr->next;
#endif
			}
		} //while
	}
	for(j=0;j<4;++j)if(j!=trump && j!=_suit) {
#ifdef reorder
		ss=3;
#endif
#ifdef inverse_after
		CurPtr=q[j].Prev;
		while(CurPtr!=&d[j]) {
#else
			CurPtr=d[j].next;
			while(CurPtr!=&q[j]) {
#endif
				if(CurPtr->who==_who1) {
					sui1[ctr1++]=CurPtr;
#ifdef reorder
					if(ss & 1) {
						sui1[ctr1-1]=sui1[j1];
						sui1[j1]=CurPtr;
						j1++;
						ss^=1;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who1);
#else
					do {CurPtr=CurPtr->next;}while(CurPtr->who==_who1);
#endif
				}
				else if(CurPtr->who==_who2) {
					sui2[ctr2++]=CurPtr;
#ifdef reorder
					if(ss & 2) {
						sui2[ctr2-1]=sui2[j2];
						sui2[j2]=CurPtr;
						j2++;
						ss^=2;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who2);
#else
					do {CurPtr=CurPtr->next;}while(CurPtr->who==_who2);
#endif
				}
				else {
#ifdef inverse_after
					CurPtr=CurPtr->Prev;
#else
					CurPtr=CurPtr->next;
#endif
				}
			} //while
		}
