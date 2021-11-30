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
#include <share.h>//_SH_DENYWR
#include <unistd.h> //usleep
#include <sys/stat.h>//stat

#include "BridgeCommon.h"
#include "Deal.h"
#include "Permutations.h"

bool signalFileExists(){return false;}

const char PLAYER_CHAR[] = "nesw";

using T=std::pair<int,double>;
using VT=std::vector<T>;
bool cmpLess(T const &a, T const &b) { return a.second < b.second; }//old name "cmp"
bool cmpGreater(T const &a, T const &b) { return a.second > b.second; }//old name "cmp1"

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

//#define BRIDGE_TEST

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
	VString vs;
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

				t=loadProblem("c:/slovesno/eclipse/bridge/problems/solvealldeals.bts",nt?vproblemsNT:vproblems);
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
		loadProblem("c:/slovesno/eclipse/bridge/problems/solvealldeals.bts", 1,0,false,1);
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

const char SHARED_FILE_NAME[]="shared.txt";

const int SEARCH_MOVES_PARAMETERS_TRUMP=1;
const int SEARCH_MOVES_PARAMETERS_NT=2;
const int SEARCH_MOVES_PARAMETERS_MISERE=3;
/* special mode for searching moves parameters can be defined in moves.bat
 * SEARCH_MOVES_PARAMETERS not defined - other modes
 * SEARCH_MOVES_PARAMETERS=1 - only trump problems
 * SEARCH_MOVES_PARAMETERS=2 - only no trump & non misere problems
 * SEARCH_MOVES_PARAMETERS=3 - only misere problems
 */
#define SEARCH_MOVES_PARAMETERS 1
//#define SEARCH_MOVES_PARAMETERS 2

/* TYPE 0 - count nodes
 * TYPE 1 - compare old and new algorithm or count for one of the algorithms
 * TYPE 2 - generate function headers/bodies for class Bridge/Preferans
 * TYPE 3 - sort moves file
 */
#if defined(SEARCH_MOVES_PARAMETERS)
#define TYPE 1
#else
#define TYPE 1
#endif

#if TYPE==3
#include <fstream>
#endif

/* SOLVE_TYPE 0 184 756 positions
 * SOLVE_TYPE 1 20 000 positions
 */
//#if SEARCH_MOVES_PARAMETERS==1
//#define SOLVE_TYPE 1
//#elif SEARCH_MOVES_PARAMETERS==2
//#define SOLVE_TYPE 0
//#else
//#define SOLVE_TYPE 1
//#endif
#define SOLVE_TYPE 0

/* FP=0 old and new algorithms
 * FP=1 only new algorithm
 */
const int FP=1;

/* 1 - full with results
 * 2 - short table
 * 3 - short table headers
 */
#define OUTPUT_TYPE 2

/* 0 - all problems
 * 1 - only trump problems
 * 2 - only no trump & non misere problems
 * 3 - only misere problems
 */
//const int PROBLEM_TYPE=0;
#if defined(SEARCH_MOVES_PARAMETERS)
const int PROBLEM_TYPE=SEARCH_MOVES_PARAMETERS;
#else
const int PROBLEM_TYPE=0;
#endif

const int MAX_PROBLEM=10;

//#define SHOW_ACCELERATION
const bool WRITE_TO_FILE=0;

#ifdef SEARCH_MOVES_PARAMETERS
	#define ONLY_LONG_PROBLEMS
#else
	//#define ONLY_LONG_PROBLEMS
#endif

//after SOLVE_TYPE is defined
#include "DealData.h"

#if TYPE==0
enum {
	SPADES,HEARTS,DIAMONDS,CLUBS
};
const char SUITS_CHAR[] = "shdcn";//Base.h
#elif TYPE==1

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


void printDealDataFromFile(const char* path);
void proceedOFiles();

bool isBridge(){
#ifdef BRIDGE_TEST
	return true;
#else
	return false;
#endif
}

#ifdef SEARCH_MOVES_PARAMETERS
int getUpper(){
	if(SEARCH_MOVES_PARAMETERS==SEARCH_MOVES_PARAMETERS_TRUMP){
		return MOVES_ONE_SUIT_OPTIONS
				* (isBridge() ? 1 : MOVES_ONE_SUIT_OPTIONS)
				* MOVES_MANY_SUITS_OPTIONS * MOVES_MANY_SUITS_OPTIONS;
	}
	else if(SEARCH_MOVES_PARAMETERS==SEARCH_MOVES_PARAMETERS_NT || SEARCH_MOVES_PARAMETERS==SEARCH_MOVES_PARAMETERS_MISERE){
		return MOVES_ONE_SUIT_OPTIONS * MOVES_MANY_SUITS_OPTIONS_NT
				* MOVES_MANY_SUITS_OPTIONS_NT;
	}
	else{
		assert(0);
		return 0;
	}
}

std::string getSearchTypeString(){
	//PROBLEM_TYPE = SEARCH_MOVES_PARAMETERS
	if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_TRUMP) {
		return "trump";
	} else if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_NT) {
		return "nt";
	} else if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_MISERE) {
		return "misere";
	}else {
		return "unknown";
	}
}

std::string getOutputFileName(int thread){
	std::string s;
#ifdef BRIDGE_TEST
	s = "bridge";
#else
	s = "preferans";
#endif
	return s + getSearchTypeString()+std::to_string(thread)+".txt";
}
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

	nodes = PREFERANS_SOLVE_ALL_DEALS_POSITIONS;
	#ifdef PREFERANS_NODE_COUNT
	#error "undef PREFERANS_NODE_COUNT in Preferans.h"
	#endif
	const char* qq[]={"all problems","only trump","only nt","only misere"};

	sa=format("nodes=%s %s %s", toString(nodes, ',').c_str(), FP==1 ?"only new algorithm":"old+new algorithms"
			,qq[PROBLEM_TYPE]
	);

#ifdef ONLY_LONG_PROBLEMS
	sa+=", only long";
#endif

	sa+=" "+getSearchTypeString()+"_params(";
	if (PROBLEM_TYPE == SEARCH_MOVES_PARAMETERS_TRUMP) {
		sa += format("%d,%d",
				PREFERANS_ORDER_FIRST_MOVE,
				PREFERANS_ORDER_OTHER_MOVES
				);
	}
	else if (PROBLEM_TYPE == SEARCH_MOVES_PARAMETERS_NT) {
		sa += format("%d,%d",
				PREFERANS_ORDER_FIRST_MOVE_NT,
				PREFERANS_ORDER_OTHER_MOVES_NT
				);
	}
	else if (PROBLEM_TYPE == SEARCH_MOVES_PARAMETERS_MISERE) {
		sa += format("%d,%d",
				PREFERANS_ORDER_FIRST_MOVE_MISERE,
				PREFERANS_ORDER_OTHER_MOVES_MISERE
				);
	}
	sa+=")\n";

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

FILE* openSharedFile(const char* fileName,const char *mode){
	FILE*f;
	double seconds=.05;
	unsigned microseconds=seconds*1000*1000;
	while( (f=_fsopen(fileName,mode,_SH_DENYWR))==NULL && errno==EACCES){
		usleep(microseconds);
	}
	if(f==NULL){
		printf("f==NULL error%d\n",errno);
	}
	return f;
}

bool fileExists (const char* path) {
  struct stat buffer;
  return (stat (path, &buffer) == 0);
}

int getNextProceedValue(){
	auto f=openSharedFile(SHARED_FILE_NAME,"r+");
	int v;
	fscanf(f,"%d",&v);
	fseek(f,0,SEEK_SET);
	fprintf(f,"%d",v+1);
	fclose(f);
	return v;
}

//VInt parseTwoParameters(int i, int n =MOVES_MANY_SUITS_OPTIONS_NT)
VInt parseTwoParameters(int i, int n) {
	return {i%n, i/n};
}

VInt parseAllParameters(int i) {
	VInt v = parseTwoParameters(i, MOVES_MANY_SUITS_OPTIONS);
	int a = v[0];
	const int ORDER = v[1];
	if(SEARCH_MOVES_PARAMETERS==SEARCH_MOVES_PARAMETERS_TRUMP && !isBridge()){
		//like in moves.h
		v = { ORDER % MOVES_ONE_SUIT_OPTIONS, (ORDER / MOVES_ONE_SUIT_OPTIONS)
				% MOVES_ONE_SUIT_OPTIONS, ORDER / MOVES_ONE_SUIT_OPTIONS
				/ MOVES_ONE_SUIT_OPTIONS };
	}
	else{
		//like in moves.h
		v = { ORDER % MOVES_ONE_SUIT_OPTIONS, ORDER / MOVES_ONE_SUIT_OPTIONS };
	}
	v.insert(v.begin(), a);
	return v;
}

int main(int argc, char *argv[]) {
#ifdef SEARCH_MOVES_PARAMETERS
	int i,upper,thread,start;
	std::string s,s1,sa;
	double time,durationTotal=0;
	//int cores=getNumberOfCores();
	VT v;

	if(argc==1){
		upper=getUpper();
		printl(upper,getSearchTypeString());
//		printl(PREFERANS_SOLVE_ALL_DEALS_POSITIONS);
//		fflush(stdout);
//		time=routine(true);
//		printl(time);
		proceedOFiles();
		return 0;
	}

	if(argc!=3){
		printl("error argc!=3");
		return 0;
	}

	thread=atoi(argv[1]);
	start=atoi(argv[2]);
	upper=getUpper();
	s=getSearchTypeString();
	printv(s,thread,start,upper)

	if(!fileExists(SHARED_FILE_NAME)){
		printl("error shared file not exists");
		return 0;
	}

	preventThreadSleep();

	while ( (i=getNextProceedValue()) < upper ) {
		auto p = parseTwoParameters(i,
				SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_TRUMP ?
						MOVES_MANY_SUITS_OPTIONS : MOVES_MANY_SUITS_OPTIONS_NT);
		if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_TRUMP) {
			PREFERANS_ORDER_FIRST_MOVE = p[0];
			PREFERANS_ORDER_OTHER_MOVES = p[1];
		} else if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_NT) {
			PREFERANS_ORDER_FIRST_MOVE_NT = p[0];
			PREFERANS_ORDER_OTHER_MOVES_NT = p[1];
		} else if (SEARCH_MOVES_PARAMETERS == SEARCH_MOVES_PARAMETERS_MISERE) {
			/*
			 0 <= PREFERANS_ORDER_FIRST_MOVE_MISERE < OM_NT
			 0 <= PREFERANS_ORDER_OTHER_MOVES_MISERE < O1*OM_NT
			 */
			PREFERANS_ORDER_FIRST_MOVE_MISERE = p[0];
			PREFERANS_ORDER_OTHER_MOVES_MISERE = p[1];
		}else{
			assert(0);
		}

		//time=randomDouble(0,100);
		time=routine(true);

		v.push_back({i,time});
		durationTotal+=time;
		/* for i+1-start steps take durationTotal, average time for one step durationTotal/(i+1-start)
		 * total steps upper-start so left upper-i-1 steps or durationTotal*(upper-i-1)/(i+1-start)
		 * not need to divide on number of cores
		 */
		s=secondsToString(durationTotal*(upper-i-1)/(i+1-start));
		s1=secondsToString(time);
		sa=format("i=%d %.3lf time %s, left %s\n",i,time,s1.c_str(),s.c_str());
		printf(sa.c_str());
		fflush(stdout);

		s=getOutputFileName(thread);
		FILE*f=fopen(s.c_str(),"a");
		fprintf(f,sa.c_str());
		fclose(f);
	}
	printv(i,upper)


#else
	printDealDataFromFile("c:/Users/user/git/bridge/bridge/bridge/problems/preferansRu.bts");
//	printDealDataFromFile("c:/Users/user/git/bridge/bridge/bridge/problems/solvealldeals.pts");
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

//output file problems to DealData array strings like { "T987.T987.#98.*Q*J", MISERE, 0, "solvealldeals1-1", false },
void printDealDataFromFile(const char* _path){
	std::string s,deal,q;
	DealData d;
	d.longProblem=true;
	MapStringString map;
	MapStringString::iterator it;
	int i,j,k,absentIndex,playerIndex;
	VInt vSpecialCards;
	bool absentCards[32];
	char absent;
	const char*p;
	VString vs,v1;
	s=_path;
	std::string path=replaceAll(s, "/", "\\");
	std::ifstream in(path);
	assert(in.good());

	auto clearAbsentCards=[&absentCards](){
		for(auto& a:absentCards){
			a=true;
		}
	};

	clearAbsentCards();

	while(std::getline(in,s)){
		if(s[0]=='%'){
			continue;
		}

		if(s.empty()){
			it = map.find ("absent");
			absent=it==map.end()?'s':it->second[0];
			absentIndex=indexOf(tolower(absent),PLAYER_CHAR);
			assert(absentIndex!=-1);

			s=map["player"];
			assert(!s.empty());
			playerIndex=indexOf(tolower(s[0]),PLAYER_CHAR);

			assert(absentIndex!=playerIndex);

			s=map["deal"];
			assert(!s.empty());
			vs=split(s);
//			out=vs[0]=="2";
			for(i=1;i<int(vs.size());i++){
				j=0;
				for(p=vs[i].c_str();*p;p++){
					char c=*p;
					if(c=='.'){
						j++;
						assert(j<4);
					}
					else{
						k=indexOf(tolower(c),RANK);
						assert(k!=-1);
						absentCards[j*8+k]=false;
					}
				}
			}
			for(i=0;i<32;i++){
				if(absentCards[i]){
					vSpecialCards.push_back(i);
//					printzn(i,' ',RANK[i%8],SUITS_CHAR[i/8]);
				}
			}
			if(vSpecialCards.size()!=2){//10cards
				goto l1539;
			}

			//can be turns=0 && play=CQ it's ok
			s=map["turns"];
			assert(!s.empty());
			if(s=="0"){
				i=-1;
			}
			else{
				assert(s=="1");
				s=map["play"];
				assert(!s.empty());
				i=indexOf(tolower(s[0]),SUITS_CHAR);
				assert(i<4 && i!=-1);
				j=indexOf(tolower(s[1]),RANK);
				assert(j<8 && j!=-1);
				i=i*8+j;
			}
			vSpecialCards.push_back(i);

			i=playerIndex<absentIndex ? playerIndex : playerIndex-1;
			assert(i+1<int(vs.size()));
			s=vs[i+1];
			v1=split(s,'.');
			assert(v1.size()==4);
			//add vSpecialCards
			i=0;
			for(auto a:vSpecialCards){
				if(a==-1 && i==2){
					break;
				}
				j=a/8;//suit
				assert(j>=0 && j<int(v1.size()));
				int card=a%8;
				char c=toupper(RANK[card]);
				std::string & q=v1[j];
				j=indexOf(c,q);
				if(i==2){
					assert(j!=-1);
					q=q.substr(0, j)+"#"+q.substr(j);
				}
				else{
					assert(j==-1);
					k=0;
					for(auto a:q){
						if(a!='*' && indexOf(tolower(a),RANK)>card){
							break;
						}
						k++;
					}
					q = q.substr(0, k) + "*" + (c + q.substr(k));
				}

				i++;
			}
			d.deal=joinV(v1,'.');


			s=map["contract"];
			assert(!s.empty());
			//mizer old files support
			d.trump = s == "misere" || s=="mizer" ?
					MISERE :
					(s.substr(1) == "NT" ? NT : indexOf(tolower(s[1]), SUITS_CHAR));

			d.comment=getFileInfo(path, FILEINFO::SHORT_NAME)+" "+vs[0];

			s=map["play"];
			if(s.length()==1){
				if(s=="N"){
					i=0;
				}
				else if(s=="E"){
					i=1;
				}
				else if(s=="W"){
					i=2;
				}
				else{
					assert(0);
				}
			}
			else{
				j=indexOf(tolower(s[0]),SUITS_CHAR);
				assert(j>=0 && j<4);
				for(i=1;i<int(vs.size());i++){
					v1 = split(vs[i], '.');
					assert(v1.size() == 4);
					if (indexOf(s[1], v1[j]) != -1) {
						i--;//starts from 1 so i--
						break;
					}
				}
			}
			d.first=i;

			printl(d.cppString());


//			for(auto [a,b]:map){
//				printl(a,"#",b)
//			}
//			printl("next problem");
//			printi;
l1539:
			map.clear();
			clearAbsentCards();
			vSpecialCards.clear();

			//break;
		}
		else{
			auto p=s.find(' ');
			map[s.substr(0, p)]=s.substr(p+1);

			//printzi("#",s.substr(0, p),"#",s.substr(p+1),"#");
		}
	}
}

#ifdef SEARCH_MOVES_PARAMETERS
void proceedOFiles(){
	std::string s;
	int i,j;
	double vd;
	VT v;

	for(i=0;i<6;i++){
		s=getOutputFileName(i);
		std::ifstream f(s);
		if(!f.is_open()){
			printl("error cann't open file",s);
			break;
		}

		while( std::getline( f, s ) ){
			if(sscanf(s.c_str(),"i=%d %lf",&j,&vd)!=2){
				printl("error");
			}
			v.push_back({j,vd});
//			break;
		}
	}

	std::sort(v.begin(), v.end(), [](auto &a, auto &b) {
		return a.first < b.first;
	});

	j=int(v.size());
	for(i=0;i<j;i++){
		if(v[i].first!=i){
			printi
			break;
		}
	}
	if(i==j){
		//shared is ok
		if(i==getUpper()){
			printan("counts are finished");
		}
		else{
			printan("run.bat",i);
		}
	}
	else{
		//need to modify o_i & shared.txt
		printan("need to modify o_i & shared.txt");
		printan("run.bat",std::min(i,j),i,j);
	}

	std::sort(v.begin(), v.end(), [](auto &a, auto &b) {
		return a.second < b.second;
	});

//	printl(map.size())
	i=0;
	const int V=3;
	for (const auto& a : v) {
		if(i<V || v.size()-i-1<V){
			s = joinV(parseAllParameters(a.first), ',');
			println("%3d (%s) %7.3lf", a.first, s.c_str(), a.second);
		}
		else if(i==V){
			printl("...")
		}
		i++;
	}
}
#endif

