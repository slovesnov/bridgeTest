#ifdef reorder	
j1=0; //dont remove!
#endif
if(trump!=NT) {
#ifdef inverse_trump
	CurPtr=q[trump].Prev;
	while(CurPtr!=&d[trump]) {
#else
		CurPtr=d[int(trump)].next;
		while(CurPtr!=&q[int(trump)]) {
#endif
			if(CurPtr->who==_who) {
#ifdef reorder
				j1=1;
#endif
				sui[ctr++]=CurPtr;
#ifdef inverse_trump
				do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who);
#else
				do {CurPtr=CurPtr->next;}while(CurPtr->who==_who);
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
		ss=1;
#endif
#ifdef inverse_after
		CurPtr=q[j].Prev;
		while(CurPtr!=&d[j]) {
#else
			CurPtr=d[j].next;
			while(CurPtr!=&q[j]) {
#endif
				if(CurPtr->who==_who) {
					sui[ctr++]=CurPtr;
#ifdef reorder
					if(ss) {
						sui[ctr-1]=sui[j1];
						sui[j1]=CurPtr;
						j1++;
						ss=0;
					}
#endif
#ifdef inverse_after
					do {CurPtr=CurPtr->Prev;}while(CurPtr->who==_who);
#else
					do {CurPtr=CurPtr->next;}while(CurPtr->who==_who);
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
