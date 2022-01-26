/*
 * endgame.h
 *
 *  Created on: 12.01.2022
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

//endgame database

#ifndef ENDGAME_H_
#define ENDGAME_H_

#include <set>
#include <thread>
#include <atomic>

#include "pcommon.h"
#include "DealData.h"
#include "BigUnsigned.h"

namespace endgame{

bool isBridge(int i) { return i < 2; };

bool isMisere(int i) { return i == 4; };

EndgameType getOption(int i) { return (i==1 || i==3) ? EndgameType::TRUMP : EndgameType::NT;};

int totalPositions (bool bridge) {
	int i=(bridge ?1:2)*BridgePreferansBase::suitLengthVector(bridge,EndgameType::NT).size()+BridgePreferansBase::suitLengthVector(bridge,EndgameType::TRUMP).size();
	return i*(bridge?Bridge::endgameCN:Preferans::endgameCN);
};

void createBinFiles(bool bridge){
	const int n = BridgePreferansBase::endgameGetN(bridge);
	const int cn = bridge?Bridge::endgameCN:Preferans::endgameCN;
	int i,j,l,st;
	char byte;
	std::string s,s1,s2;
	int steps[5];
	for(i=0;i<5;i++){
		const bool bridge=isBridge(i);
		const EndgameType option=getOption(i);
		steps[i]=BridgePreferansBase::suitLengthVector(bridge, option).size();
	}

//	double d=log2(n+1);
//	const int bits=int(d)+(int(d)!=d);

	const int chunkbits=n<=3?2:4;
	//printl(chunkbits)
	const char* T[]={"nt", "trump","misere"};
	const char t=(bridge?'b':'p');

	for (j = 0; j < 2+(!bridge); j++) {
		st=steps[j+(bridge?0:2)];
		s1=t+std::to_string(n);
		std::ofstream f( s1+T[j]+".bin",std::ofstream::binary);
		//need TODO if (chunkbits*st*cn)%8!=0
		if((chunkbits*st*cn)%8!=0){
			printl("todo totalbits%8!=0");
			return;
		}

		l=0;
		byte=0;
		for (i = 0; i < st; i++) {
			s =s1+"/"+T[j]+std::to_string(i)+".bin";
			if(!fileExists(s.c_str())){
				printl(s);
				return;
			}
			assert(fileExists(s.c_str()));

			s=fileGetContent(s, true);
			for(auto k:s){
				if(!(k>=0 && k<=n)){
					printl(k,n);
				}
				assert(k>=0 && k<=n);
				byte |= k<<l;
				l+=chunkbits;
				if(l==8){
					f.write(&byte, 1);
					l=0;
					byte=0;
				}
			}
		}
	}
}

std::atomic_int atom;

void createEndgameFilesThread(int thread){
	int i,j,k,sc[4];
	VVInt v;
	Bridge br;
	Preferans pr;
	CARD_INDEX ci[52];
	std::string s,s1,s2;
	Permutations p[3];
	char e;

	clock_t begin;
//	int count=0;

	auto getStepOptions=[](int i,int steps[5]) {
		int j;
		for(j=0;j<5;j++){
			if(i<steps[j]){
				break;
			}
			i-=steps[j];
		}
		return std::make_pair(j,i);
	};

	int steps[5],upper=0;
	for(i=0;i<5;i++){
		const bool bridge=isBridge(i);
		const EndgameType option=getOption(i);
		steps[i]=BridgePreferansBase::suitLengthVector(bridge, option).size();
		upper+=steps[i];
//		printl(i,steps[i])
	}
	preventThreadSleep();

	while ((j = atom++) < upper ) {
		begin=clock();
		auto pa = getStepOptions(j, steps);
		i = pa.first;

		const bool bridge = isBridge(i);
		const EndgameType option = getOption(i);
		const bool misere = !bridge && isMisere(i);
		const int step = pa.second;
		s1=option == EndgameType::TRUMP ? "trump" :(misere?"misere": "nt");

		const int n = BridgePreferansBase::endgameGetN(bridge);
		const int ntotal = BridgePreferansBase::endgameGetN(bridge ,true);

		for (i = 0; i < 3; i++) {
			p[i].init(n, ntotal - n * i, COMBINATION);
		}

		s2=format("%c%d", bridge ? 'b' : 'p',n);
		s = s2+'\\'+s1+std::to_string(step)+".bin";
		std::ofstream file(s, std::ofstream::binary);

		auto len = BridgePreferansBase::suitLengthVector(bridge, option)[step];
		for (auto &p0 : p[0]) {
			for (auto &p1 : p[1]) {
				for (auto &p2 : p[2]) {
					k=BridgePreferansBase::bitCode(bridge,p0,p1,p2);

					//get sc[]
					for (i = 0; i < 4; k >>= 2 * len[i], i++) {
						//2^(2*len[i])-1
						sc[i] = k & ((1 << (2 * len[i])) - 1);
					}


					//c [0-12 - spades A-2], [13-25 hearts A-2], [26-38 diamonds A-2], [39-51 clubs A-2]
					for (i = 0; i < 52; i++) {
						ci[i] = CARD_INDEX_ABSENT;
					}

					for (i = 0; i < 4; i++) {
						k = sc[i];
						for (j = 0; j < len[i]; (k >>= 2), j++) {
							ci[13 * i + j] = CARD_INDEX(
									CARD_INDEX_NORTH + (k & 3));
						}
					}

					const bool trumpChanged = true;
					const CARD_INDEX first = CARD_INDEX_NORTH;
					const int trump = option == EndgameType::TRUMP ? 0 : NT;
					if (bridge) {
						br.solveEstimateOnly(ci, trump, first, trumpChanged);
						e=char(br.m_e);
						file.write(&e, 1);

					} else {
						CARD_INDEX preferansPlayer[] = { CARD_INDEX_NORTH,
								CARD_INDEX_EAST, CARD_INDEX_SOUTH };
						for(i=0;i<SIZEI(preferansPlayer);i++){
							/* change order preferansPlayer[] because it's more convenient
							 * for getting estimate see line i+=w[t2]; in pi2.h file
							 */
							pr.solveEstimateOnly(ci, trump, first, preferansPlayer[i==0?0:(i==1?2:1)], misere,
									preferansPlayer, trumpChanged);
							e=char(pr.m_e);
							file.write(&e, 1);
						}
					}

//					count++;
//					if (count % 500 == 0) {
//						double o = timeElapse(begin) / count
//								* totalPositions(bridge) / 3600.
//								/ getNumberOfCores();
//						println("%s %d %.2lf, avgFull %.2lf (hours)",
//								s1.c_str(), count,
//								timeElapse(begin), o)
//						fflush(stdout);
//					}
				}
			}
		}
		file.close();
		//break;

		s = format("t%d %s %s %d/%d time ", thread,
				bridge ? "bridge" : "preferans", s1.c_str(), step,
				steps[pa.first]) + secondsToString(timeElapse(begin));
		printl(s)
		fflush(stdout);

	}

}


void createEndgameFiles(){
	int i;
	std::vector<std::thread> vt;
	const int threads = getNumberOfCores()-1;
	atom=0;
	auto begin=clock();

#if defined(BRIDGE_ENDGAME) || defined(PREFERANS_ENDGAME)
	printl("error BRIDGE_ENDGAME or PREFERANS_ENDGAME is defined");
	return;
#endif

	char c[3];
	c[2]=0;
	for (i = 0; i < 2; i++) {
		bool bridge=i==0;
		c[0]=bridge ? 'b' : 'p';
		c[1]='0'+BridgePreferansBase::endgameGetN(bridge);
		mkdir(c);
	}

	println("run %d thread(s)",threads)
	fflush(stdout);


	for (i = 0; i < threads; ++i) {
		vt.push_back(std::thread(createEndgameFilesThread, i));
	}

	for (auto& a : vt){
		a.join();
	}


	createBinFiles(true);
	createBinFiles(false);
	printl("the end total time "+secondsToString(timeElapse(begin)));

}

void bridgeSpeedTest(bool ntproblems){
	preventThreadSleep();
	Bridge br;
	clock_t begin;
	double t,tt=0;
	int i=0;
	const int from=3;
	const int to=10;
	assert(to>=from);
	//printl(bridgeDeals[ntproblems].size())
	for(i=from;i<to;i++){
		auto &a=bridgeDeals[ntproblems][i];
		begin = clock();
		br.solveFull(a.c, a.m_trump, a.m_first, true);
		t=timeElapse(begin);
		tt+=t;

		printl(i,t,br.m_e)
		fflush(stdout);
	}

	printl("total time",tt)
	//fflush(stdout);
}

void preferansSpeedTest(){
	preventThreadSleep();
	int i;
	int r[RESULT_SIZE];
	clock_t begin;
	double t,tt;
	std::string s,s1;

	const bool type = 1;

	if (type == 0) {
		Preferans pr;
		i = 0;
		for (auto &d : preferansDeal) {
			pr.solveEstimateOnly(d.c, d.m_trump, d.m_first, d.m_player,
					d.m_misere, PREFERANS_PLAYER, true);
			assert(pr.m_e==preferansDealE[i]);
			printl(i,pr.m_e)
			;
			i++;
		}
		return;
	}

#ifdef PREFERANS_ENDGAME
	printl("PREFERANS_ENDGAME is defined",PREFERANS_SOLVE_ALL_DEALS_POSITIONS);
#else
	printl("PREFERANS_ENDGAME is not defined",PREFERANS_SOLVE_ALL_DEALS_POSITIONS);
#endif
	//printl(PREFERANS_SOLVE_ALL_DEALS_POSITIONS);
	fflush(stdout);

	tt=0;
	for (i=0;i<SIZEI(dealData);i++) {
		begin = clock();
		run(PREFERANS_SOLVE_ALL_DEALS_POSITIONS, i, false,r);
		t = timeElapse(begin);
		tt += t;
		s=JOINS(results[i],',');
		s1=JOINS(r,',');
		if(s==s1){
			printan(i,t,"ok");
		}
		else{
			printan(i,t,"error",s,s1);
		}
		fflush(stdout);
	}

	printl(tt);
}

//need uint64_t type for showTablesBP()
uint64_t getBinomialCoefficient(int k,int n){
	uint64_t r = 1;
	int i;
	/* for big n,k
	 * C(n,k)=n*C(n-1,k-1)/k
	 * C(n,k)=n*C(n-1,k-1)/k=(n/k)*(n-1/k-1)...(n-k+1/1)C(n-k,0); C(n-k,0)=1
	 */
	for (i = 1; i <= k; i++) {
		r *= n - k + i;
		r /= i;
	}
	return r;
}

void showTablesBP(){
	int i, j;
	int n, a[3];
	VVInt v;
	for (i = 0; i < 2; i++) {
		for (n = 1;; n++) {
			bool bridge = i == 0;
			for (j = 0; j < 3; j++) {
				v = BridgePreferansBase::suitLengthVector(n, bridge, EndgameType(j));
				if (v.empty()) {
					goto l144;
				}
				if (!j) {
					printf("\n<tr><td>%d", n);
				}
				a[j] = v.size();
				printf("<td>%d</td>", a[j]);
			}
			printf("<td>%d", (bridge ? 1 : 2) * a[1] + a[2]);
		}
		l144: ;
		printf("\n");
	}
}

void showTablesBP1(){
 	const int digits=6;
 	const bool latex=0;
	int i,j,l;
	BigUnsigned r,r1;
	std::string s,s1;

	auto cm=[](int l,bool bridge){
		BigUnsigned i=1;
		const int k=bridge?4:3;
		for(int j=0;j<k-1;j++){
			i*=getBinomialCoefficient(l, (k -j)*l);
		}
		return i;
	};

	auto double2HtmlString=[](double v){
		auto s=format("%.1le",v);
		auto p=s.find('e');
		assert(s[p+1]=='+');
		int i=2+(s[p+2]=='0');
		return s.substr(0,p)+"&sdot;10<sup>"+s.substr(p+i)+"</sup>";
	};

	for(j=0;j<2 ;j++){
		const bool bridge=j==0;
		printl(bridge?"bridge":"preferans")
		for(l=1;l<=(bridge?13:10);l++){
			i=(bridge?1:2)*BridgePreferansBase::suitLengthVector(l,bridge,EndgameType::NT).size()+BridgePreferansBase::suitLengthVector(l,bridge,EndgameType::TRUMP).size();
			r=cm(l, bridge);
			r1=r*i;
			s=r1.toString();
			double rd=r1.toDouble();
			if(latex){
				prints(" & ",l,s,(r*i).toString(digits,',')+format(" $\\approx%c.%c \\cdot 10^{%d}$",s[0],s[1],int(s.length()-1)))
				printan("\\\\")
			}
			else{
				s1 =formats("</td><td>", l,
						(l>11?"<small>":"")+toString(i)+"&sdot;"+r.toString(digits, ',')+" = "+ r1.toString(digits, ',')
						+"  &asymp; "+double2HtmlString(rd))+(l>11?"</small>":"");
				s1 ="<tr><td>"+ s1+"</td>";
				//(l>3 ? 2:4) l<=3 sufficient two bits, otherwise three bits
				double v = r1.toDouble() * (bridge?1:3)/(l>3 ? 2:4);
				const char *q[] = { "", "k", "m", "g", "t", "p", "e", "z", "y" };
				for ( i = 0; i < SIZEI(q); i++) {
					if (v < 1024) {
						break;
					}
					v /= 1024.;
				}
				s1 += "<td>" + format("%.2lf %sb", v*(i==SIZEI(q) ? 1024 :1), q[i==SIZEI(q) ? i-1:i]) + "</td>";
				printzn(s1)
			}
		}
	}
}

void test(){

}

void routine(){

	createBinFiles(1);
	createBinFiles(0);
//	createEndgameFiles();//multithread with union bin files create
//	bridgeSpeedTest(0);
//	bridgeSpeedTest(1);
//
//	preferansSpeedTest();


//	test();

/*
	createEndgameFiles();//multithread with union bin files create
	showTablesBP();
	showTablesBP1();
	bridgeSpeedTest(0);
	bridgeSpeedTest(1);
	createBinFiles(true);
	createBinFiles(false);
*/

/*
	Bridge br;
	auto &a = bridgeDeals[1][20];
	br.solveFull(a.c, a.m_trump, a.m_first, true);

*/

}


}//namespace

#endif /* ENDGAME_H_ */
