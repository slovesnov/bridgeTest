//// enhansed transposition cutoff's

#ifdef ENHANCED
#define PG  alpha4=next->gP(alpha2-1)+1;
#define MG  alpha4=~next->gP(~alpha3);

bool mask[13]= {true,true,true,true,true,true,true,true,true,true,true,true,true};
bool further_count=true;
alpha3=alpha1;
for(j3=0;j3<counter_suitable_cards3;++j3) {

	p3=suitable_t3[j3];
	Remove(p3)
	if(compare(_suitcard,p1->suitcard)) { // _> 1
		if(compare(_suitcard,p2->suitcard)) { // _>1 && _>2
			if(compare(_suitcard,p3->suitcard)) { //_
				next->_who_ =_who_;
				next->whoIndex=whoIndex;
				PG
			}
			else { //3
				next->whoIndex=whoIndex+3;
				next->_who_=whoArray[next->whoIndex];
				MG
			}
		}
		else { // 2 > _ && 2 > 1
			if(compare(p2->suitcard,p3->suitcard)) { //2
				next->whoIndex=whoIndex+2;
				next->_who_=whoArray[next->whoIndex];
				PG
			}
			else { //3          
				next->whoIndex=whoIndex+3;
				next->_who_=whoArray[next->whoIndex];
				MG
			}
		}
	}
	else { //1 > _
		if(compare(p1->suitcard,p2->suitcard)) { // 1>_ && 1>2
			if(compare(p1->suitcard,p3->suitcard)) { //1
				next->whoIndex=whoIndex+1;
				next->_who_=whoArray[next->whoIndex];
				MG
			}
			else { //3
				next->whoIndex=whoIndex+3;
				next->_who_=whoArray[next->whoIndex];
				MG
			}
		}
		else { // 2 > _ && 2 > 1
			if(compare(p2->suitcard,p3->suitcard)) { //2
				next->whoIndex=whoIndex+2;
				next->_who_=whoArray[next->whoIndex];
				PG
			}
			else { //3          
				next->whoIndex=whoIndex+3;
				next->_who_=whoArray[next->whoIndex];
				MG
			}
		}
	}
	Restore(p3)
	if(alpha4!=101 && alpha4!=-101) {
		mask[j3]=false; //old_start version false
		if(alpha4<alpha3) {
#ifdef bestmove4
			bs=p3->suit;
			bc=p3->card;
#endif
			alpha3=alpha4;
			further_count=false; //old_start version false
			break;
		}
	} //if(alpha4...
} // for
#undef MG
#undef PG
if(further_count && next->next!=NULL) {
#endif //ENHANCED
//// enhansed transposition cutoff's
#define PG  alpha4=next->g(alpha2-1)+1;
#define MG  alpha4=~next->g(~alpha3);

#ifndef ENHANCED
	alpha3=alpha1;
#endif
	for(j3=0;j3<counter_suitable_cards3;++j3)
#ifdef ENHANCED
	if(mask[j3])
#endif
	{
		p3=suitable_t3[j3];
		Remove(p3)
#ifdef bestmove4
		//5aug2014 int II=1;
#endif
		if(compare(_suitcard,p1->suitcard)) { // _> 1
			if(compare(_suitcard,p2->suitcard)) { // _>1 && _>2
				if(compare(_suitcard,p3->suitcard)) { //_
					next->_who_ =_who_;
					next->whoIndex=whoIndex;
					PG
				}
				else { //3
					next->whoIndex=whoIndex+3;
					next->_who_=whoArray[next->whoIndex];
					MG
				}
			}
			else { // 2 > _ && 2 > 1
				if(compare(p2->suitcard,p3->suitcard)) { //2
					next->whoIndex=whoIndex+2;
					next->_who_=whoArray[next->whoIndex];
					PG
				}
				else { //3          
					next->whoIndex=whoIndex+3;
					next->_who_=whoArray[next->whoIndex];
					MG
				}
			}
		}
		else { //1 > _
			if(compare(p1->suitcard,p2->suitcard)) { // 1>_ && 1>2
				if(compare(p1->suitcard,p3->suitcard)) { //1
					next->whoIndex=whoIndex+1;
					next->_who_=whoArray[next->whoIndex];
					MG
				}
				else { //3
					next->whoIndex=whoIndex+3;
					next->_who_=whoArray[next->whoIndex];
					MG
				}
			}
			else { // 2 > _ && 2 > 1
				if(compare(p2->suitcard,p3->suitcard)) { //2
					next->whoIndex=whoIndex+2;
					next->_who_=whoArray[next->whoIndex];
					PG
				}
				else { //3          
					next->whoIndex=whoIndex+3;
					next->_who_=whoArray[next->whoIndex];
					MG
				}
			}
		}
		Restore(p3)
#ifdef bestmove4
		//5aug 2014 II=2;
#endif
		if(alpha4<alpha3) {
#ifdef bestmove4
			bs=p3->suit;
			bc=p3->card;
#endif
			alpha3=alpha4;
			break;
		}
	} // for
#undef MG
#undef PG

#ifdef ENHANCED
}
#endif
