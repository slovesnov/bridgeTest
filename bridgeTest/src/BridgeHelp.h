/*
 * BridgeHelp.h
 *
 *  Created on: 04.12.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef BRIDGEHELP_H_
#define BRIDGEHELP_H_

const char PLAYER_CHAR[] = "nesw";

struct BridgeProblemInfo{
	std::string file;//source file path
	int index;//index of problem in source file
	double time;//duration to solve problem

	/* arrays copy correctly
	 * https://stackoverflow.com/questions/5700204/c-does-implicit-copy-constructor-copy-array-member-variable
	 * so not need to redefine copy/move operators
	 */
	//parameters as it passed to solveFull function
	CARD_INDEX c[52];
	int trump;
	CARD_INDEX first;

	static const std::string LEADER[];

	std::string toDealString() const {
		assert(first >= CARD_INDEX_NORTH && first <= CARD_INDEX_WEST);
		return "Deal({" + JOINS(c, ',') + "}, "
				+ (nt() ? std::string("NT") : std::to_string(trump))
				+ ", CARD_INDEX_" + LEADER[first - CARD_INDEX_NORTH] + ") /*"
				+ file + " " + std::to_string(index) + "*/";
	}

	bool nt() const {
		return trump == NT;
	}
};
const std::string BridgeProblemInfo::LEADER[4] = { "NORTH", "EAST", "SOUTH", "WEST" };
using VBridgeProblemInfo = std::vector<BridgeProblemInfo>;

/*
 * trumpOption=0 all problems
 * trumpOption=1 only trump
 * trumpOption=2 only no trump
 *
 * type=0 solve new+old
 * type=1 solve new only
 * type=2 solve old only
 * type=3 some test
 */
double loadBridgeProblems(const char *fn, int begin, int end, int type,
		bool verbose, bool outputonlylong, double minTime, int trumpOption = 0,VBridgeProblemInfo*vpi=0) {
	const int MAX_END=10000;
	if(end==-1){
		end=begin;
	}
	if(end==0){
		end=MAX_END;
	}

	char b[256];
	char a[256];
	char *p;
	const char*pc,*p1;
	int i,j,n,eo;
	CARD_INDEX cf;
	clock_t start;
	double t,tt=0,to,tto=0;
	int trump=-1,first=-1;
	Bridge br;
	BridgeProblemInfo bpi;
	bpi.file=getFileInfo(fn,FILEINFO::NAME);

	FILE*f=fopen(fn,"r");
	if(!f){
		printf("[%s]",fn);
	}
	assert(f);

	char so[256];

	if(verbose){
		if(begin==end){
			printf("file %s %d\n",fn,begin);
		}
		else{
			printf("file %s %d-%d\n",fn,begin,end);
		}
		fflush(stdout);
	}

	const char* z[]={"contract ","play "};
	for( n=begin ; n< end+1 ; n++ ){
		bpi.index=n;
		sprintf(a,"deal %d ",n);
		while((pc=fgets(b,256,f))!=0){
			if(strncmp(b,a,strlen(a))==0){
				break;
			}
		}
		if(!pc){
			if(end!=MAX_END){
				printf("problem not found");
			}
			break;
		}
		j=-1;
		sprintf(a,"%s",b+strlen(a));
		for (i = 0; i < 2; i++) {
			fgets(b, 256, f);
			assert(strlen(b) + 1 > strlen(z[i]));
			assert(strncmp(b, z[i], strlen(z[i])) == 0);
			pc = i == 0 ? SUITS_CHAR : PLAYER_CHAR;
			p1=b + strlen(z[i]) + 1 - i;
			p = strchr(pc, tolower(*p1));
			//printf("--%d %c %d %x %d\n",i,tolower(*p1),int(p1[1]),p, int(i==1 && !p) );
			if((i==1 && !p) || (i==1 && strchr(RANK, tolower(p1[1])) ) ){
				pc=SUITS_CHAR;
				p = strchr(pc, tolower(*p1));
				assert(p);
				j=(p - pc)*13;

				pc=RANK;
				p = strchr(pc, tolower(p1[1]));
				assert(p);
				j += p - pc;
			}
			else{
				if (!p) {
					printf("%d[%s]", i, b);
				}
				assert(p);
				first = p - pc;
			}
			if (i == 0) {
				trump = first;
			}
		}
		fflush(stdout);

		if( (trumpOption==1 && trump==NT) || (trumpOption==2 && trump!=NT)){
			continue;
		}

		cf=CARD_INDEX(first+1);
		Deal d(a, trump, cf);
		if(j!=-1){
			cf=d.c[j];
			first=int(d.c[j])-1;
			d.c[j]=CARD_INDEX(cf+4);
		}

		bool pr=0;

		if(type==0 || type==1){
			start = clock();
			br.solveFull(d.c, trump, cf, true);

			t = double(clock() - start) / CLOCKS_PER_SEC;
			tt+=t;

			bpi.time=t;
			bpi.trump=trump;
			bpi.first=cf;
			std::copy(std::begin(d.c), std::end(d.c), std::begin(bpi.c));

			//problem number/cards e time/total_time
			if(verbose){
				sprintf(so,"%2d e=%2d time=%5.2lf trump=%d",n,br.m_e,t,trump);
				if (vpi) {
					vpi->push_back( bpi);
				}
//				sprintf(so,"%2d e=%2d time=%5.2lf move=%c%c trump=%d",n,br.m_e,t,RANK[br.m_best%13],SUITS_CHAR[br.m_best/13],trump);
				if (!outputonlylong || t > minTime) {
					pr=1;
					printf(so);
				}
			}

		}

		if(type==0 || type==2){
			start = clock();
			BridgePosition::solve( (int*)d.c, trump, first,true);
			t=0;
			to = double(clock() - start) / CLOCKS_PER_SEC;
			tto+=to;
			if(j==-1){//no first move
				eo= first%2==0 ? BridgePosition::northSouthTricks : BridgePosition::eastWestTricks;
			}
			else{//1 card on table, only this case now
				eo= first%2==1 ? BridgePosition::northSouthTricks : BridgePosition::eastWestTricks;
			}

			//problem number/cards e time/total_time, olde time/total_time
			if (!outputonlylong || to > minTime || (type==0 && t>minTime )) {
				if(t<=minTime && !pr){
					printf(so);
				}
				if(type==0){
					printf(",");
				}
				printf(" old e=%2d time=%5.2lf %s move=%c%c trump=%d\n", eo, to
						,to==t?"same time " : (to>t?"old slower":"new slower")
						,RANK[12-int(BridgePosition::bc)],SUITS_CHAR[int(BridgePosition::bs)],trump);
				pr=0;
			}
			if(type==0 && eo!=br.m_e){
				println("error e=%d old=%d",br.m_e,eo);
				exit(1);
				break;
			}
		}
		if(pr){
			printf("\n");
		}
		fflush(stdout);
	}//for n
	fclose(f);

	if(type==3){
		printf("total slow fast %.3lf %.3lf\n",tto,tt);//slow fast
	}

	if(verbose){
		printf("total");
		if(type==0 || type==1){
			printf(" new %.2lf",tt);
		}
		if(type==0 || type==2){
			printf(" old %.2lf",tto);
		}
		printf("\n");
	}
	return tt;
}

double loadBridgeProblems(const char *fn, int begin, int end = -1,
		bool movesOptimization = false, int trumpOption = 0) {
	const int type = movesOptimization ? 1 : 0;
	const bool verbose = !movesOptimization;

	const bool outputonlylong = 0;
	const double minTime = 1;

	return loadBridgeProblems(fn, begin, end = -1, type, verbose,
			outputonlylong, minTime, trumpOption);
}

//solve all bridgeDeals[index] and return total time, analog of routine() function for preferans
double bridgeRoutine(int index){

	Bridge br;
	clock_t start;
	double t,tt=0;
//	int i=0;
	for(auto a:bridgeDeals[index]){
		start = clock();
		br.solveFull(a.c, a.m_trump, a.m_first, true);
		t=double(clock() - start) / CLOCKS_PER_SEC;
		tt += t;

//		printl(i,t)
//		fflush(stdout);
//		i++;
	}
	return tt;
}


#endif /* BRIDGEHELP_H_ */
