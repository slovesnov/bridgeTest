/* https://stackoverflow.com/questions/3479163/eclipse-c-include-error-no-such-file-or-directory
 * Project > Properties > C/C++ General > Paths and Symbols > Includes
 * use one solver for two projects bridge & bridgeTest
 *
 */
#include <cstdio>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cassert>

#include "Deal.h"
#include "Permutations.h"

bool signalFileExists(){return false;}

const char PLAYER_CHAR[] = "nesw";

using T=std::pair<std::string,double>;
using VT=std::vector<T>;
bool cmp(T const &a, T const &b) { return a.second < b.second; }
bool cmp1(T const &a, T const &b) { return a.second > b.second; }

using VS=std::vector<std::string>;

int BRIDGE_ORDER_FIRST_MOVE=2;
int BRIDGE_ORDER_OTHER_MOVES=36;

int BRIDGE_ORDER_FIRST_MOVE_NT=0;
int BRIDGE_ORDER_OTHER_MOVES_NT=0;


int PREFERANS_ORDER_FIRST_MOVE= 0;
int PREFERANS_ORDER_OTHER_MOVES= 13;

int PREFERANS_ORDER_FIRST_MOVE_NT=0;
int PREFERANS_ORDER_OTHER_MOVES_NT=0;

int PREFERANS_ORDER_FIRST_MOVE_MISERE= 4;
int PREFERANS_ORDER_OTHER_MOVES_MISERE= 2;

#define BRIDGE_TEST

#ifdef BRIDGE_TEST

#include "Bridge.h"
#include "old/BridgePosition.h"

/* TYPE 0 - search bridge parameters
 * TYPE 1 - solve file
 * TYPE 2 - sort moves file
 * TYPE 3 - output list of long problems
 */
#define TYPE 1

/*
 * trumpOption=0 all problems
 * trumpOption=1 only trump
 * trumpOption=2 only no trump
 */
double loadProblem(const char*fn,int begin,int end=-1,bool movesOptimization=false,int trumpOption=0,VInt*numbers=0){
	//type=0 solve new+old
	//type=1 solve new only
	//type=2 solve old only
	//type=3 some test
	const int type = movesOptimization ? 1 : 0;
	const bool verbose=!movesOptimization;

	const bool outputonlylong=0;
	const double minTime=1;


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
	int i,j,n,nn,eo;
	CARD_INDEX cf;
	clock_t start;
	double t,tt=0,to,tto=0;
	int trump=-1,first=-1;
	Bridge br;
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
	for( nn=numbers?0:begin ; nn< (numbers?int(numbers->size()):end+1) ; nn++ ){
		n=numbers?(*numbers)[nn]:nn;
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
			//problem number/cards e time/total_time
			if(verbose){
				sprintf(so,"%2d e=%2d time=%5.2lf move=%c%c",n,br.m_e,t,RANK[br.m_best%13],SUITS_CHAR[br.m_best/13]);
				if (!outputonlylong || t > minTime) {
					pr=1;
					printf(so);
				}
			}

		}

		if(type==0 || type==2){
			start = clock();
			BridgePosition::solve( (int*)d.c, trump, first,true);
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

double loadProblem(const char*fn,VInt& numbers){
	return loadProblem(fn,0,-1,true,0,&numbers);
}


#if TYPE==2 || TYPE==3

#include <iostream>
#include <fstream>

/* total new moves 79.02, old moves 65.21
 * old 46.53 with quick & sure tricks, 60.48 without quick & sure tricks
 *
 * */

void sortfiles(){
	std::string s;
	std::string tm = "time=";
	std::string files[]={"oldmoves.txt","newmoves.txt","newmoves_oldWithoutSureAndQuick.txt","oldmoves_oldWithoutSureAndQuick.txt"};
	VT a;
	VS vs;
	for(auto file:files){
		vs.clear();
		a.clear();

		std::ifstream input( file );
		while( getline( input, s ) ){
			auto p=s.find(tm);
			if(p==std::string::npos){
				vs.push_back(s);
			}
			else{
				auto t=atof(s.c_str()+p+tm.length());
				a.push_back({s,t});
			}
		}

		std::sort(a.begin(), a.end(),cmp1 );

		auto p=file.find('.');
		s=file.substr(0,p)+"_o.txt";
		std::ofstream o( s );

		for(auto&v:vs){
			o<<v<<std::endl;
		}
		for(auto&v:a){
			o<<v.first<<std::endl;
		}
	}
}

void longProblemsLists(){
	std::string s;
	std::string tm = "time=";
	std::string file="oldmoves.txt";
	std::ifstream input( file );
	VT a;
	int j;
	VInt v[2];
	while( getline( input, s ) ){
		auto p=s.find(tm);
		if(p!=std::string::npos){
			auto t=stod(s.substr(p+tm.length()));
			if(t>1){
				printf("%s\n",s.c_str());
				v[s.find("trump=4")!=std::string::npos].push_back(stoi(s));
			}
		}
	}

	for(j=0;j<2;j++){
		printf("VInt vproblems%s=",j==1?"NT":"");
		bool f=1;
		for(int i:v[j]){
			printf("%c%d",f?'{':',',i);
			f=0;
		}
		printf("};\n");
	}
}

#endif

int main() {

#if TYPE==0
	//TODO
	#ifndef NEW_MOVES_ORDER
#error NEW_MOVES_ORDER not defined
	#endif

		VT v;
		std::string s;
		double t;
		VInt vproblems={7,18,21,22,24,27};
		VInt vproblemsNT={8,9,12,13,14,15,17,19};
		int i,j;
		bool nt=true;
		const int om=nt?MOVES_MANY_SUITS_OPTIONS_NT:MOVES_MANY_SUITS_OPTIONS;

		for(i=0;i<om;i++){
			if(nt){
				BRIDGE_ORDER_FIRST_MOVE_NT=i;
			}
			else{
				BRIDGE_ORDER_FIRST_MOVE=i;
			}
			for(j=0;j<MOVES_ONE_SUIT_OPTIONS*om;j++){
				if(nt){
					BRIDGE_ORDER_OTHER_MOVES_NT=j;
				}
				else{
					BRIDGE_ORDER_OTHER_MOVES=j;
				}

				t=loadProblem("c:/slovesno/eclipse/bridge/problems/solveallfoe.bts",nt?vproblemsNT:vproblems);
				//t=loadProblem("GeorgeCoffin.bts",vp, 1, 6,true,1);//1 68
	//			t=loadProblem("GeorgeCoffin.bts",vp, 1, 6,true,1);//1 68
				if(nt){
					s=format("(%d,%d)",BRIDGE_ORDER_FIRST_MOVE_NT,BRIDGE_ORDER_OTHER_MOVES_NT);
				}
				else{
					s=format("(%d,%d)",BRIDGE_ORDER_FIRST_MOVE,BRIDGE_ORDER_OTHER_MOVES);
				}
				v.push_back({s,t});
				printf("%s %.2lf\n",s.c_str(),t);
				fflush(stdout);
			}
		}

		printf("ordered\n");

		FILE*f=fopen("o.txt","w+");

		std::sort(v.begin(), v.end(),cmp );
		for(auto& a:v){
			fprintf(f,"%s %.2lf\n",a.first.c_str(),a.second);
			printf("%s %.2lf\n",a.first.c_str(),a.second);
		}
		fclose(f);
#elif TYPE==1

	BridgePosition::allocateTables();

	int t=0;
	if(t==0){
		//loadProblem("ra.bts", 1);
		//loadProblem("#1.bts", 1);
		//double loadProblem(const char*fn,int begin,int end=-1,bool movesOptimization=false,int trumpOption=0,VInt*numbers=0){
		loadProblem("c:/slovesno/eclipse/bridge/problems/solveallfoe.bts", 1,0,false,1);
//			loadProblem("GeorgeCoffin.bts", 1, 666,false,0);//2 68
	}
	else{
		loadProblem("#1.bts", 1);
//		loadProblem("#2.bts", 1);
//		loadProblem("#3.bts", 1);
//		loadProblem("ra.bts", 1);
//		loadProblem("GeorgeCoffin.bts", 1, 68);//2 68
	}

	BridgePosition::freeTables();

	printf("the end");
#elif TYPE==2
	sortfiles();
#else
	longProblemsLists();
#endif
}

#else //not BRIDGE_TEST

#include "Preferans.h"
#include "old/PreferansOld.h"

/* special mode for searching moves parameters can be defined in moves.bat
 * SEARCH_MOVES_PARAMETERS=1 search non misere problems
 * SEARCH_MOVES_PARAMETERS=2 search misere problems
 * SEARCH_MOVES_PARAMETERS not defined - other modes
 */
//#define SEARCH_MOVES_PARAMETERS 2

/* TYPE 0 - count nodes
 * TYPE 1 - compare old and new algorithm or count for one of the algorithms
 * TYPE 2 - generate function headers/bodies for class Bridge/Preferans
 * TYPE 3 - sort moves file
 */
#if defined(SEARCH_MOVES_PARAMETERS)
#define TYPE 1
#else
#define TYPE 2
#endif

#if TYPE==3
#include <fstream>
#endif

/* SOLVE_TYPE 0 184 756 positions
 * SOLVE_TYPE 1 20 000 positions
 */
#if SEARCH_MOVES_PARAMETERS==1
#define SOLVE_TYPE 1
#elif SEARCH_MOVES_PARAMETERS==2
#define SOLVE_TYPE 0
#else
#define SOLVE_TYPE 1
#endif

/* FP=0 old and new algorithms
 * FP=1 only new algorithm
 */
const int FP=1;

/* 1 - full with results
 * 2 - short table
 * 3 - short table headers
 */
#define OUTPUT_TYPE 3

/* 0 - all problems
 * 1 - only trump problems
 * 2 - only no trump & non misere problems
 * 3 - only misere problems
 */
#if defined(SEARCH_MOVES_PARAMETERS)
const int PROBLEM_TYPE=SEARCH_MOVES_PARAMETERS;
#else
const int PROBLEM_TYPE=0;
#endif

const int MAX_PROBLEM=10;

//#define SHOW_ACCELERATION
const bool WRITE_TO_FILE=true;

#ifdef SEARCH_MOVES_PARAMETERS
#define ONLY_LONG_PROBLEMS
#else
//#define ONLY_LONG_PROBLEMS
#endif


#if TYPE==0
enum {
	SPADES,HEARTS,DIAMONDS,CLUBS
};
const char SUITS_CHAR[] = "shdcn";//Base.h
#elif TYPE==1

struct DealData{
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

};

/* Note problem1 from preferansRu is not used because each player
 * has only seven cards. 4 deals from solveallfoe file, and 6 from preferansRu
 *
 * eclipse formatting make dealData array ugly after automatic format so leave normal version here
const DealData dealData[]={
	{"T987.T987.#98.*Q*J",5,0,"solveallfoe0"},
	{"T987.T987.*98.*Q#J",5,0,"solveallfoe1"},
	{"#A*8*7.AKT.KJ.A987",3,0,"solveallfoe2"},
	{"AQJ*T8*7.KT8.KJ.9",0,2,"solveallfoe3"},

	{ "QT7.KQJ.A*8*7.KQJ", 0, 2, "preferansRu0" },
	{ "A*8*7.AKT.KJ.A987", 3, 2, "preferansRu2" },
	{ "KQT7.AQJ.KJ*9.A*J", 0, 2, "preferansRu3" },
	{ "T987.98.987.*A*K8", 5, 2, "preferansRu4" },
	{ "*K*Q8.T987.987.98", 5, 2, "preferansRu5" },
	{ "A*98.AJ7.AJ8.KT*9", 4, 2, "preferansRu6" }
};

const int results[][RESULT_SIZE] = {
		{103936, 13832, 612, 9362, 35612, 20754, 648, 0, 0, 0, 0},
		{8544, 147660, 1340, 9050, 15156, 2876, 130, 0, 0, 0, 0},
		{0, 0, 0, 0, 1016, 19625, 104962, 58993, 160, 0, 0},
		{0, 0, 0, 3015, 134666, 46917, 158, 0, 0, 0, 0},
		{441, 4547, 18204, 78510, 78762, 4292, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1376, 29432, 100086, 53802, 60, 0, 0},
		{0, 0, 0, 0, 1226, 75131, 104670, 3699, 30, 0, 0},
		{80407, 977, 23345, 49263, 22306, 8207, 251, 0, 0, 0, 0},
		{80407, 977, 23345, 49263, 22306, 8207, 251, 0, 0, 0, 0},
		{0, 0, 0, 102170, 76768, 5816, 2, 0, 0, 0, 0}
};

const int results[][RESULT_SIZE] = {
		{7978, 2338, 233, 2062, 5021, 2308, 60, 0, 0, 0, 0},
		{906, 13958, 366, 2054, 2384, 325, 7, 0, 0, 0, 0},
		{0, 0, 0, 0, 226, 1643, 11300, 6799, 32, 0, 0},
		{0, 0, 0, 0, 12740, 7222, 38, 0, 0, 0, 0},
		{0, 229, 828, 9058, 9807, 78, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 523, 4578, 9569, 5318, 12, 0, 0},
		{0, 0, 0, 0, 398, 14262, 5173, 167, 0, 0, 0},
		{3345, 454, 4914, 7402, 3117, 742, 26, 0, 0, 0, 0},
		{6259, 241, 2728, 5349, 3523, 1774, 126, 0, 0, 0, 0},
		{0, 0, 0, 10895, 8338, 767, 0, 0, 0, 0, 0}
};

 */
const DealData dealData[]={
	{"T987.T987.#98.*Q*J",5,0,"solveallfoe0",0},//misere
	{"T987.T987.*98.*Q#J",5,0,"solveallfoe1",0},//misere
	{"#A*8*7.AKT.KJ.A987",3,0,"solveallfoe2",0},
	{"AQJ*T8*7.KT8.KJ.9",0,2,"solveallfoe3",1},

	{ "QT7.KQJ.A*8*7.KQJ", 0, 2, "preferansRu0",0 },
	{ "A*8*7.AKT.KJ.A987", 3, 2, "preferansRu2",0 },
	{ "KQT7.AQJ.KJ*9.A*J", 0, 2, "preferansRu3",1 },
	{ "T987.98.987.*A*K8", 5, 2, "preferansRu4",1 },//misere
	{ "*K*Q8.T987.987.98", 5, 2, "preferansRu5",1 },//misere
	{ "A*98.AJ7.AJ8.KT*9", NT, 2, "preferansRu6",1 }
};

const int RESULT_SIZE = 11;

#if SOLVE_TYPE==0
const int PREFERANS_SOLVE_ALL_FOE_POSITIONS=184756;
const int results[][RESULT_SIZE] = {
		{103936, 13832, 612, 9362, 35612, 20754, 648, 0, 0, 0, 0},
		{8544, 147660, 1340, 9050, 15156, 2876, 130, 0, 0, 0, 0},
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
const int PREFERANS_SOLVE_ALL_FOE_POSITIONS = 20000;
const int results[][RESULT_SIZE] = {
		{7978, 2338, 233, 2062, 5021, 2308, 60, 0, 0, 0, 0},
		{906, 13958, 366, 2054, 2384, 325, 7, 0, 0, 0, 0},
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

void run(int nodes, int problem, bool old,int*result);
#else
void generate ();
#endif

#if TYPE!=2
const CARD_INDEX PREFERANS_PLAYER[] = {
		CARD_INDEX_WEST,
		CARD_INDEX_NORTH,
		CARD_INDEX_EAST };

#endif

double routine(bool movesOptimization=false) {
	double tt=0;
#if TYPE==0
#ifndef PREFERANS_NODE_COUNT
#error "define PREFERANS_NODE_COUNT in Preferans.h"
#endif
	Deal a[]= {
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
	const int e[]= {10,7,7,7,4, 8,8, 7};
	const int SIZE=sizeof(e)/sizeof(e[0]);
	static_assert(SIZE==sizeof(a)/sizeof(a[0]));
	int i,j,k;

	struct Result {
		double time;
		int nodes;
		int best;
	}res[SIZE*2];

	int n[]= {0,0};
	double t[]= {0,0};
	clock_t sp;
#define T0_OLD

	for(k=j=0;j<2;j++) {
		i=0;

#ifdef T0_OLD
		PreferansOld
#else
		Preferans
#endif
		position(j==0);
		for(auto& d:a) {
			sp = clock();
#ifdef T0_OLD
			if(d.m_misere) {
				position.solvebMisere(d.c,d.m_trump,d.m_first,d.m_player,PREFERANS_PLAYER,true);
			}
			else {
				position.solveb(d.c,d.m_trump,d.m_first,d.m_player,PREFERANS_PLAYER,true);
			}
#else
			if(d.m_misere) {
				position.solvebMisere(d.c,d.m_trump,d.m_first,d.m_player,PREFERANS_PLAYER,true);
			}
			else if(d.m_trump==NT) {
				position.solvebNT(d.c,d.m_trump,d.m_first,d.m_player,PREFERANS_PLAYER,true);
			}
			else {
				position.solveb(d.c,d.m_trump,d.m_first,d.m_player,PREFERANS_PLAYER,true);
			}
#endif
			if(e[i++]!=position.m_e) {
				printf("error");
				break;
			}

			Result& r=res[k++];
			r.time = double(clock() - sp) / CLOCKS_PER_SEC;
			r.nodes=position.m_nodes;
			r.best=position.m_best;
		}
	}

	printf("-time-----nodes-best---time-----nodes-best\n");
	for(i=0;i<SIZE;i++) {
		for(j=0;j<2;j++) {
			k=j*SIZE+i;
			Result& r=res[k];
			auto s=intToString(r.nodes);
			printf("%.3lf %10s %c%c   ",r.time,s.c_str(),RANK[r.best%13],SUITS_CHAR[r.best/13]);
			t[j]+=r.time;
			n[j]+=r.nodes;
		}
		printf("\n");
	}
	printf("---smallHash---------------bigHash--------\n");
	for(j=0;j<2;j++) {
		auto s=intToString(n[j]);
		printf("%.3lf %10s      ",t[j],s.c_str());
	}
	printf("\nPreferans%s",
#ifdef T0_OLD
		"Old"
#else
		""
#endif
	 );

#elif TYPE==1
	double t;
	int i,j;
	clock_t start;
	int nodes = 1;
	int problem = 1;
	std::string s,q,s1,sa,w;
	double tn;//tt defined upper
	int r[RESULT_SIZE];
	bool verbose=!movesOptimization;

	nodes = PREFERANS_SOLVE_ALL_FOE_POSITIONS;
	#ifdef PREFERANS_NODE_COUNT
	#error "undef PREFERANS_NODE_COUNT in Preferans.h"
	#endif
	const char* qq[]={"all problems","only trump","only nt","only misere"};

	sa=format("nodes=%s %s %s", intToString(nodes, ',').c_str(), FP==1 ?"only new algorithm":"old+new algorithms"
			,qq[PROBLEM_TYPE]
	);

#ifdef ONLY_LONG_PROBLEMS
	sa+=", only long";
#endif

	if (PROBLEM_TYPE == 1) {
		sa += format(" trump_params(%d,%d)",
				PREFERANS_ORDER_FIRST_MOVE,
				PREFERANS_ORDER_OTHER_MOVES
				);
	}
	else if (PROBLEM_TYPE == 2) {
		sa += format(" nt_params(%d,%d)",
				PREFERANS_ORDER_FIRST_MOVE_NT,
				PREFERANS_ORDER_OTHER_MOVES_NT
				);
	}
	else if (PROBLEM_TYPE == 3) {
		sa += format(" misere_params(%d,%d)",
				PREFERANS_ORDER_FIRST_MOVE_MISERE,
				PREFERANS_ORDER_OTHER_MOVES_MISERE
				);
	}
	sa+="\n";

	if(verbose){
		printf("%s",sa.c_str());
		fflush(stdout);
	}


#ifdef SHOW_ACCELERATION
	double ti[2]={3.19};
#endif

	for (i = FP; i < 2; i++) {

		if(FP==0){
			printf(	i==0 ?"old algorithm\n" : "new algorithm\n"		);
			fflush(stdout);
		}
#if OUTPUT_TYPE==3
		if(verbose){
			printf("  time n/sec avgfull(min) comment\n");
		}
#endif

		tt = 0;
		tn = 0;
		for (problem = 0; problem < MAX_PROBLEM; problem++) {
			DealData const&d=dealData[problem];
			if(PROBLEM_TYPE==1){
				if(d.misere() || d.nt()){
					continue;
				}
			}
			else if(PROBLEM_TYPE==2){
				if(d.misere() || !d.nt()){
					continue;
				}
			}
			else if(PROBLEM_TYPE==3){
				if(!d.misere()){
					continue;
				}
			}


#ifdef ONLY_LONG_PROBLEMS
			if(!d.longProblem){
				continue;
			}
#endif

			start = clock();
			run(nodes, problem, i == 0,r);
			t = double(clock() - start) / CLOCKS_PER_SEC;
			tt += t;
			tn += nodes;

			if(d.misere()){
				s1="misere";
			}
			else if(d.nt()){
				s1="nt";
			}
			else{
				s1="trump"+std::to_string(d.trump);
			}

			if (results[problem][0] == -1) {
				q= "??";
			}
			else {
				for(j=0;j<RESULT_SIZE && results[problem][j]==r[j];j++);
				q= j==RESULT_SIZE ? "ok" : "error";
				if(j!=RESULT_SIZE){
					printl(q);
					for(j=0;j<RESULT_SIZE;j++){
						printf("%d %d\n",results[problem][j], r[j]);
					}
					return 1;
				}

#if OUTPUT_TYPE==1
				s="";
				for(j=0;j<RESULT_SIZE;j++){
					s+=forma(r[j]);
					s+=' ';
				}
#endif
			}

#if OUTPUT_TYPE==1

			w=format("%*stime=%6.2lf %5.0lfn/sec avgfull=%6.2lfmin %s %s\n",
#if SOLVE_TYPE==0
					46
#else
					40
#endif
					,s.c_str(), t, nodes / t, t * 184756 / nodes / 60,
					dealData[problem].comment.c_str(),
					q.c_str());

#elif OUTPUT_TYPE==2

			w=format("time=%6.2lf %5.0lfn/sec avgfull=%6.2lfmin %s %s\n",
					 t, nodes / t, t * 184756 / nodes / 60,
					dealData[problem].comment.c_str(),
					q.c_str());
#else
			w=format("%6.2lf %5.0lf %6.2lf %s %6s %s\n",
					 t, nodes / t, t * 184756 / nodes / 60,
					dealData[problem].comment.c_str(),
					s1.c_str(),q.c_str());

#endif
			sa += w;
			if(verbose){
				printf("%s",w.c_str());
				fflush(stdout);
			}
		}
#ifdef SHOW_ACCELERATION
		ti[i]=tt;
#endif
		w=format("%s fulltime=%.2lf, overall %.0lfn/sec\n",
				i == 0 ? "old algorithm" : "new algorithm", tt, tn / tt);
		sa += w;

		if(verbose){
			printf("%s",w.c_str());
			fflush(stdout);
		}

		if(WRITE_TO_FILE){
			FILE*f=fopen("o.txt","a");
			fprintf(f,"%s\n",sa.c_str());
			fclose(f);
		}
	}
#ifdef SHOW_ACCELERATION
	printf("acceleration %.1lf%% (from %.2lf)\n",(1-ti[1]/ti[0])*100,ti[0]);
#endif


#elif TYPE==2
	generate();
#elif TYPE==3

	std::ifstream infile("om.txt");
	std::ofstream ofile("oms.txt");
	std::string line,o;
	int l;
	VT v;
	double t;
	size_t f;
	const std::string FT="fulltime=";
	const std::string F1="only long ";

	for (l=1;std::getline(infile, line);l++){
		f=line.find(F1);
		if(f!=std::string::npos){
			o=line.substr(f+F1.length());
			//printl(line.substr(64))
		}
		f=line.find(FT);
		if(f!=std::string::npos){
			t=atof(line.substr(f+FT.length()).c_str());
			//printl(line.substr(f),t)
			v.push_back({o,t});
		}
	}

	std::sort(v.begin(), v.end(), cmp );

	for (T&a:v){
		o=format("%10s %.2lf\n",a.first.c_str(),a.second);
		ofile<<o;
		printf(o.c_str());
	}

	printl("ok");

#endif

	return tt;
}

int main() {
	VT v;
	std::string s;

#ifdef SEARCH_MOVES_PARAMETERS
	double t;

	for(int i=0;i<MOVES_MANY_SUITS_OPTIONS;i++){

		if (PROBLEM_TYPE == 1) {
			//0-MOVES_MANY_SUITS_OPTIONS
			//int preferans_order_first_move= 0;

			//0-MOVES_ONE_SUIT_OPTIONS*MOVES_ONE_SUIT_OPTIONS*MOVES_MANY_SUITS_OPTIONS
			//int preferans_order_other_moves= 13;
			PREFERANS_ORDER_FIRST_MOVE=i;
		}
		else{
			//0-MOVES_MANY_SUITS_OPTIONS
			//int  preferans_order_first_move_misere= 4;

			//0-MOVES_ONE_SUIT_OPTIONS*MOVES_MANY_SUITS_OPTIONS
			//int preferans_order_other_moves_misere= 2;
			PREFERANS_ORDER_FIRST_MOVE_MISERE= i;
		}


		t=routine(true);
		if (PROBLEM_TYPE == 1) {
			s = format("(%d,%d)",
					PREFERANS_ORDER_FIRST_MOVE,
					PREFERANS_ORDER_OTHER_MOVES
					);
		}
		else{
			s = format("misere(%d,%d)",
					PREFERANS_ORDER_FIRST_MOVE_MISERE,
					PREFERANS_ORDER_OTHER_MOVES_MISERE
					);
		}
		v.push_back({s,t});
		printf("%s %.2lf\n",s.c_str(),t);
		fflush(stdout);
	}

	printf("ordered\n");

	FILE*f=fopen("o.txt","w+");

	std::sort(v.begin(), v.end(),cmp );
	for(auto& a:v){
		fprintf(f,"%s %.2lf\n",a.first.c_str(),a.second);
		printf("%s %.2lf\n",a.first.c_str(),a.second);
	}
	fclose(f);

#else
	routine();
#endif
}

#if TYPE==1

std::string parseDeal(const std::string& deal, int firstmove, int cards[10],
		int absent[2], int sorted[12], int& leadCard) {
	int suit = 0;
	bool out, lead;
	int i;
	int*c = cards;
	int*a = absent;
	int*pi;
	const char*p, *q;

	leadCard = -1;

	for (p = deal.c_str(); *p != 0; p++) {
		if (*p == '.') {
			if (suit == 3) {
				return "too many dots";
			}
			suit++;
			continue;
		}
		out = *p == '*';
		lead = *p == '#';
		if (out || lead) {
			p++;
		}
		q = strchr(RANK, tolower(*p));
		if (!q) {
			printf("[%c%c%d]", *p, tolower(*p), int(*p));
			return "invalid symbol";
		}
		if (q - RANK >= 8) { //'6'-'2'
			return "invalid card";
		}
		if (out) {
			if (a - absent == 2) {
				return "too many absent cards";
			}
			pi = a++;
		}
		else if (lead) {
			if (firstmove != 0) {
				return "error lead card found when player is not north";
			}
			if (leadCard != -1) {
				return "too many absent cards";
			}
			pi = &leadCard;
		}
		else {
			if (c - cards == 10) {
				return "too many player cards";
			}
			pi = c++;
		}
		*pi = suit * 13 + q - RANK; //in bridge project
	}
	if (a - absent != 2) {
		return "too few absent cards";
	}

	const int pcards = c - cards;
	if ((leadCard != -1 && pcards != 9) || (leadCard == -1 && pcards != 10)) {
		return "too few player cards";
	}

	for (i = 0; i < pcards; i++) {
		sorted[i] = cards[i];
	}
	for (; i < pcards + 2; i++) {
		sorted[i] = absent[i - pcards];
	}
	if (leadCard != -1) {
		sorted[i++] = leadCard;
	}

	std::sort(sorted, sorted + i);
	for (i--; i > 0; i--) {
		if (sorted[i] == sorted[i - 1]) {
			return "same card appears two or more times";
		}
	}

	return "";
}

void run(int nodes, int problem, bool old,int*result) {

	int i, j, k;
	CARD_INDEX ptr[52];
	int cards[10], absent[2], sorted[12], lead, o[20];
	const int min = 0;
	const int max = nodes;
	Preferans position(false);
	PreferansOld positionOld(false);

	for (i = 0; i < RESULT_SIZE; i++) {
		result[i] = 0;
	}

	//check params at first
	DealData const&d=dealData[problem];

	const int trump = d.nt() ?NT:d.trump;
	const bool misere = d.misere();
	i = d.first;
	const int firstmove = i == 2 ? 3 : i;
	std::string s = parseDeal(d.deal, i, cards, absent, sorted, lead);
	if (s.length() != 0) {
		return;
	}

	for (i = j = k = 0; i < 52; i++) {
		if (i % 13 >= 8) { //skip 2,3,4,5,6
			continue;
		}
		if (j < 12 && sorted[j] == i) {
			j++;
			continue;
		}
		assert(k<20);
		o[k++] = i;
	}
	assert(k==20);

	for (i = 0; i < (lead == -1 ? 10 : 9); i++) {
		ptr[cards[i]] = CARD_INDEX_NORTH;
	}
	for (i = 0; i < 2; i++) {
		ptr[absent[i]] = CARD_INDEX_ABSENT;
	}
	if (lead != -1) {
		ptr[lead] = CARD_INDEX_NORTH_INNER;
	}

	Permutations p(10, 20, COMBINATION);

	for (j = 0; j < min; j++) {
		p.next();
	}

	for (; j < max; j++, p.next()) {
		auto& pi = p.getIndexes();
		for (i = 0; i < 20; i++) {
			ptr[o[i]] = CARD_INDEX_EAST;
		}
		for (i = 0; i < p.getK(); i++) {
			ptr[o[pi[i]]] = CARD_INDEX_WEST;
		}

		//firstmove index of player
		const CARD_INDEX PLAYER[] = {
				CARD_INDEX_NORTH,
				CARD_INDEX_EAST,
				CARD_INDEX_SOUTH,
				CARD_INDEX_WEST };
		if (old) {
			positionOld.solveEstimateOnly(ptr, trump, PLAYER[firstmove],
					CARD_INDEX_NORTH, misere, PREFERANS_PLAYER, j == min);
			//here is number of whisters
			i = 10-positionOld.m_playerTricks;
		}
		else {
			position.solveEstimateOnly(ptr, trump, PLAYER[firstmove],
					CARD_INDEX_NORTH, misere, PREFERANS_PLAYER, j == min);
			//here is right
			i = position.m_playerTricks;
		}
		assert(i>=0 && i<=10);
		result[i]++;
	}

}
#elif TYPE==2
const char*anm[]={"","NT","Misere"};
const char*w[] = { "#ifdef MISERE", "#elif defined(NT)", "#else", };

void p(const char*s){
	printf("%s\n",s);
}

void ifd1(){
	const char*f = "suitableCards2";
	int i;
	for (i = 0; i < 3; i++) {
		printf("%s\n\t%s%s\n", w[i], f, anm[2 - i]);
	}
	p("#endif");

}

void ifd2(){
	const char*f = "e";
	int i;
	for (i = 0; i < 3; i++) {
		printf("%s\n", w[i]);
		printf("\t#ifdef STOREBEST\n");
		printf("\t\t#define F %sb%s\n",f,anm[2-i]);
		printf("\t#else\n");
		printf("\t\t#define F %s%s\n",f,anm[2-i]);
		printf("\t#endif\n");
	}
	p("#endif");

}

void gp(){
	p("//BEGIN AUTOMATICALLY GENERATED TEXT");

	int i,j,k;
	const char*before[]={"int","void"};
	const char*after[]={"const int* w, int a, int b","const CARD_INDEX c[52], int trump, CARD_INDEX first,\n\tCARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged"};
	const char*name[]={"e","solve"};
	const char*include[]={"pi","psolve"};

	bool b;
	for(k=0;k<2;k++){
		for(i=0;i<6;i++){
			b=i%2;
			j=i/2;
			if(i){
				p("");
			}
			printf("%s Preferans::%s%s%s(%s) {\n",before[k],name[k], b ? "b":"", anm[j],after[k] );
			if(b){
				p("#define STOREBEST");
			}
			if(j){
				printf("#define %s\n",j==1?"NT":"MISERE");
			}
			printf("#include \"%s.h\"\n",include[k]);
			if(j){
				printf("#undef %s\n",j==1?"NT":"MISERE");
			}
			if(b){
				p("#undef STOREBEST");
			}
			p("}");
		}
	}

	for(i=0;i<3;i++){
		p("");
		printf(
				"void Preferans::suitableCards2%s(int suit, const int* w, SC& c1, SC& c2) {\n",anm[i]);
		printf("\tsuitableCards%s(suit, w[1], c1);\n",anm[i]);
		printf("\tsuitableCards%s(suit, w[2], c2);\n",anm[i]);
		p("}");
	}

	p("//END AUTOMATICALLY GENERATED TEXT");

}

void gb(){
	const char*before[]={"int","void"};
	const char*after[]={"const int* w, int a","const CARD_INDEX c[52], int trump, CARD_INDEX first,\n\tbool trumpChanged, int lowTricks, int highTricks"};
	const char*name[]={"e","solve"};
	const char*include[]={"bi","bsolve"};

	p("//BEGIN AUTOMATICALLY GENERATED TEXT");
	int i,j,k;
	bool b;
	for(k=0;k<2;k++){
		for(i=0;i<4;i++){
			b=i%2;
			j=i/2;
			if(i){
				p("");
			}
			printf("%s Bridge::%s%s%s(%s) {\n",before[k],name[k], b ? "b":"", anm[j],after[k] );
			if(b){
				p("#define STOREBEST");
			}
			if(j){
				printf("#define %s\n",j==1?"NT":"MISERE");
			}
			printf("#include \"%s.h\"\n",include[k]);
			if(j){
				printf("#undef %s\n",j==1?"NT":"MISERE");
			}
			if(b){
				p("#undef STOREBEST");
			}
			if(k==0){
				p("\treturn a;");
			}
			p("}");
		}
	}

	p("//END AUTOMATICALLY GENERATED TEXT");

}


void generate () {
	gb();
}

#endif //TYPE
#endif //BRIDGE_TEST
