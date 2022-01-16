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

#include <numeric>//iota

namespace endgame{

typedef std::vector<VInt> VVInt;

int getN(bool bridge,bool total=false){
	//TODO
	int i=bridge?2:4;//number of cards of each player
	if(total){
		i*= (bridge?4:3);
	}
	return i;
};

int cm(bool bridge,int n){
	printi
	Permutations p;
	int r=1;
	int m=3+bridge;
	for(int i=0;i<m-1;i++){
		p.init(n, (m-i)*n, COMBINATION);
		r*=p.number();
	}
	return r;
}

VVInt suitLengthVector(bool bridge,EndgameType option) {
	//l[] - number of cards in suit
	int l[4];
	int ntotal=getN(bridge,true);
	const int up = std::min(bridge ? 13 : 8, ntotal);
	VVInt v;
	VInt vi;
	for (l[0] = 0; l[0] <= up; l[0]++) {
		for (l[1] = 0; l[1] <= up; l[1]++) {
			for (l[2] = 0; l[2] <= up; l[2]++) {
				l[3] = ntotal - l[0] - l[1] - l[2];
				if (l[3] >= 0 && l[3]<=up) {
					if (option == EndgameType::ALL
							|| (option == EndgameType::NT  && l[0] <= l[1] && l[1] <= l[2]
									&& l[2] <= l[3])
							|| (option == EndgameType::TRUMP && l[1] <= l[2]
									&& l[2] <= l[3])) {
						vi.assign(l, l+4);
						v.push_back(vi);
					}
				}
			}
		}
	}
	return v;
}

int bitCode(bool bridge, VInt const &p0, VInt const &p1, VInt const &p2) {
	int i,j;
	int n=getN(bridge);
	int ntotal=getN(bridge,true);
	VInt freepos(ntotal);
	std::iota(freepos.begin(), freepos.end(), 0);

	VInt vb[3];
	for (i = 0; i < 3; i++) {
		vb[i].resize(n);
	}

	const VInt *pv[] = { &p0, &p1, &p2 };
	i=0;
	for(auto q:pv){
		/* use reverse order to get more understandable vectors
		 * and need to remove from freepos in reverse order
		 * p0={0,1,2} vb[0]={2,1,0}
		 * p1={0,1,2} vb[1]={5,4,3}
		 * p2={0,1,2} vb[2]={8,7,6}
		 */
		j = n - 1;
		for (auto it = q->rbegin(); it != q->rend(); it++) {
			vb[i][j--] = freepos[*it];
			freepos.erase(freepos.begin() + *it);
		}
		i++;
	}

	// bit code v[0] - 01 bits, v[1] - 10, v[2] - 11
	int c=0;
	for(i=0;i<3;i++){
		for(j=0;j<n;j++){
			c |= (i+1)<<2*vb[i][j];
		}
	}
	return c;
}

bool isBridge(int i) { return i < 2; };

bool isMisere(int i) { return i == 4; };

auto getOption(int i) { return (i==1 || i==3) ? EndgameType::TRUMP : EndgameType::NT;};

auto cm (bool bridge) {//if bridge=1 C^n_4n*C^n_3n*C^n_2n, else C^n_3n*C^n_2n
	int i=1;
	const int n=getN(bridge);
	Permutations p;
	const int k=bridge?4:3;
	for(int j=0;j<k-1;j++){
		p.init(n, (k -j)*n, COMBINATION);
		i*=p.number();
	}
	return i;
}

auto totalPositions (bool bridge) {
	int i=(bridge ?1:2)*suitLengthVector(bridge,EndgameType::NT).size()+suitLengthVector(bridge,EndgameType::TRUMP).size();
	return i*cm(bridge);
};

void proceedFiles(){
	bool bridge=true;
	const int n = getN(bridge);
	int i,j,k,l;
	char byte;
	std::string s;
	VString v;
	int steps[5];
	for(i=0;i<5;i++){
		const bool bridge=isBridge(i);
		const EndgameType option=getOption(i);
		steps[i]=suitLengthVector(bridge, option).size();
	}

//	double d=log2(n+1);
//	const int bits=int(d)+(int(d)!=d);

	const int chunkbits=n<=3?2:4;

	//nt, trump
	//printl(steps[0],steps[1],endgameCM(bridge))

	for (j = 0; j < 2; j++) {
		std::ofstream f(format("b%s.bin",j == 0 ? "nt" : "trump"),std::ofstream::binary);
		k=chunkbits*steps[j]*cm(bridge);
		//TODO k%8!=0
		if(k%8!=0){
			printl("todo totalbits%8!=0")
		}
		printl("totalbits",k,k/8,k%8);

		l=0;
		byte=0;
		for (i = 0; i < steps[j]; i++) {
			s = format("b%s%d.txt", j == 0 ? "nt" : "trump", i);
			assert(fileExists(s.c_str()));

			s = fileGetContent(s);
			v=split(s);
			assert(int(v.size())==cm(bridge)+1);
			assert(v[v.size()-1]=="");
			v.pop_back();

			int gi=0;
			for(auto a:v){
				assert(parseString(a, k));
				parseString(a, k);
				assert(k>=-n && k<=n);
				assert(k%2==n%2);
				if(i==0 && (gi==1836 || gi==1452) ){
					printl(gi,"e=",k,(k+n)/2,l)
				}
				k=(k+n)/2;
				byte |= k<<l;
				l+=chunkbits;
				if(l==8){
					f.write(&byte, 1);
					l=0;
					byte=0;
				}
				gi++;
			}
//			printzn("[",v[v.size()-1],"[")
//			k
//			printl(parseString(d, k, radix))
//			printl(v.size())
		}
	}
	printl(n)
	printi
}

void rotate(int n,int bits,int a[3]){
	int i,j,r;

	//TODO m_w bridge class member
	int m_w[43];
	i=0;
	for(int& a:m_w){
		a=i%4;
		i++;
	}

	assert(bits % 2 == 0);
	for (j = 0; j < 3; j++) {
		r = 0;
		for (i = 0; i < bits / 2; i++) {
			r |= m_w[((n>>(2*i)) & 3)+j+1] << (2 * i);
		}
		a[j] = r;
	}
}

void speedTest(bool ntproblems){
	preventThreadSleep();
	Bridge br;
	clock_t begin;
	double t,tt=0;
	int i=0;
	for(i=3;i<10;i++){
		auto &a=bridgeDeals[ntproblems][i];
//	}
//	for(auto a:bridgeDeals[ntproblems]){
//		if(i==0){
//			i++;
//			continue;
//		}
		begin = clock();
		br.solveFull(a.c, a.m_trump, a.m_first, true);
		t=timeElapse(begin);
		tt+=t;

		printl(i,t,br.m_e)
		fflush(stdout);
		i++;
	}

	printl("total time",tt)
	fflush(stdout);
}

void routine(){

	speedTest(0);
	speedTest(1);

	{
//		proceedFiles();

/*
		int i,j,k;
		bool bridge=true;
		Permutations p[3];
		Bridge br;
		const int n = getN(bridge);
		const int ntotal = getN(bridge ,true);

		for (i = 0; i < 3; i++) {
			p[i].init(n, ntotal - n * i, COMBINATION);
		}

		int jj=0;
		auto v = suitLengthVector(true,  EndgameType::NT);
		auto len=v[0];
		int sc[4];
		for (auto &p0 : p[0]) {
			for (auto &p1 : p[1]) {
				for (auto &p2 : p[2]) {
					k=bitCode(bridge,p0,p1,p2);
					if(jj==1836 || jj==1452){
						printl(jj,binaryCodeString(k, ntotal*2))
//						println("0x%x",(k<<4)|8)
//						printl(binaryCodeString((k<<4)|8,ntotal*2+4))

						for (i = 0; i < 4; k >>= 2 * len[i], i++) {
							//2^(2*len[i])-1
							sc[i] = k & ((1 << (2 * len[i])) - 1);
//							printzn("suit", i, " code", "=",
//									binaryCodeString(sc[i],len[i]*2),
//									format(" 0x%x", sc[i]), " len=", len[i]);
						}

						CARD_INDEX ci[52];

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
						const int trump = NT;
						//br.m_e br.m_ns br.m_ew
						br.solveEstimateOnly(ci, trump, first, trumpChanged);
						printl("eold=", br.m_ns-br.m_ew,br.m_ns,br.m_ew);
						//	m_e = (m_cards + i) / 2;

						printl("enew=", br.m_e*2-br.m_cards,br.m_e,br.m_cards);


					}
					jj++;
				}
			}
		}


	return;
*/
	}

	{
		Bridge br;
	auto& a=bridgeDeals[1][0];
//	printl(a.m_trump,cm(true));
//	start = clock();
	br.solveFull(a.c, a.m_trump, a.m_first, true);
	return;
	}
//	t=double(clock() - start) / CLOCKS_PER_SEC;
/*
	int i,j,k,sc[4];
	VVInt v;
	//int* endgameLength[2];//NT+ trump
//	int*pi;
	int* endgameIndex[4];//4 number of rotates(players)
	Permutations p[3];
	clock_t begin;
	int a[3];
	const bool bridge=1;

	//Bridge class member
	int m_w[43];
	i=0;
	for(int& a:m_w){
		a=i%4;
		i++;
	}

	for (i = 0; i < 2; i++) {
		v = suitLengthVector(true, i ? EndgameType::TRUMP : EndgameType::NT);
		VInt const& max=*std::max_element(v.begin(), v.end(), [](auto &a, auto &b) {
			return a[2] < b[2];
		});

		const int size=(max[2]+1)*169;
		printl("need memory",size)
//		endgameLength[i]=pi=new int[size];
//
//		k=0;
//		for (auto a : v) {
//			j = a[0] + 13 * (a[1] + 13 * a[2]);
//			assert(j < size);
//			pi[j] = k++;
//		}
//
//		delete[]pi;
	}


	begin=clock();
	const int n = getN(bridge);
	const int ntotal = getN(bridge ,true);

	for (i = 0; i < 3; i++) {
		p[i].init(n, ntotal - n * i, COMBINATION);
	}

	const int max=16*1024*1024;
	for (i = 0; i < 4; i++) {
		endgameIndex[i]=new int[max];
	}

	printl(ntotal*2)

	j=0;
	for (auto &p0 : p[0]) {
		for (auto &p1 : p[1]) {
			for (auto &p2 : p[2]) {
				k=bitCode(bridge,p0,p1,p2);
				assert(k<max);
				endgameIndex[0][k]=j;
				rotate(k,ntotal*2,a);
				for(i=0;i<3;i++){
					endgameIndex[i+1][k]=a[i];
				}
				j++;
			}
		}
	}

	for (i = 0; i < 4; i++) {
		delete[]endgameIndex[i];
	}

	printl(timeElapse(begin))

//	proceedFiles();
	return;
*/
	int i,j,k,sc[4];
	VVInt v;
	Bridge br;
	Preferans pr;
	CARD_INDEX ci[52];
	std::string s;
	Permutations p[3];

	auto begin=clock();
	int count=0;

	const bool print=0;

//	printl(totalPositions(3,true),totalPositions(4,false))

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
		steps[i]=suitLengthVector(bridge, option).size();
		//upper+=steps[i];
		//printl(i,bridge,option==EndgameType::TRUMP? "trump":"nt",isMisere(i),steps[i])
	}

	//bridge only
	upper=steps[0]+steps[1];
//	printl(upper)

	FILE*f=fopen(SHARED_FILE_NAME,"w+");
	fprintf(f,"0");
	fclose(f);

	if(!fileExists(SHARED_FILE_NAME)){
		printl("error shared file not exists");
		return;
	}

	preventThreadSleep();

	while ((j = getNextProceedValue()) < upper) {
		auto pa = getStepOptions(j, steps);
		i = pa.first;
		const bool bridge = isBridge(i);
		const EndgameType option = getOption(i);
		const bool misere = isMisere(i);
		const int step = pa.second;
		printl(j,bridge?"bridge":"preferans",option==EndgameType::TRUMP? "trump":"nt",step)
		fflush(stdout);

		const int n = getN(bridge);
		const int ntotal = getN(bridge ,true);

		for (i = 0; i < 3; i++) {
			p[i].init(n, ntotal - n * i, COMBINATION);
		}

		s = format("_%c%s%d.txt", bridge ? 'b' : 'p',
				option == EndgameType::TRUMP ? "trump" : "nt", step);
		std::ofstream file(s);

		auto len = suitLengthVector(bridge, option)[step];
		for (auto &p0 : p[0]) {
			for (auto &p1 : p[1]) {
				for (auto &p2 : p[2]) {
					k=bitCode(bridge,p0,p1,p2);

					if (print) {
						char buff[64];
						printl(itoa(k,buff,2),binaryCodeString(k))
						;
					}
					//get sc[]
					for (i = 0; i < 4; k >>= 2 * len[i], i++) {
						//2^(2*len[i])-1
						sc[i] = k & ((1 << (2 * len[i])) - 1);
						if (print) {
							printzn("suit", i, " code", "=",
									binaryCodeString(sc[i]),
									format(" 0x%x", sc[i]), " len=", len[i]);
						}
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

					if (print) {
						s = "";
						for (i = 0; i < 52; i++) {
							auto c = ci[i];
							j = c - CARD_INDEX_NORTH;
							if (c == CARD_INDEX_ABSENT) {
								s += '-';
							} else if (j >= 0 && j < 4) {
								s += PLAYER_CHAR[j];
							} else {
								s += '?';
							}
							if ((i + 1) % 13 == 0) {
								s += ' ';
							}
						}
						printl(s)
					}
					const bool trumpChanged = true;
					const CARD_INDEX first = CARD_INDEX_NORTH;
					const int trump = option == EndgameType::TRUMP ? 0 : NT;
					if (bridge) {
						//br.m_e br.m_ns br.m_ew
						br.solveEstimateOnly(ci, trump, first, trumpChanged);
						file << br.m_ns-br.m_ew<< " ";

					} else {
						//pr.solveEstimateOnly(c, trump, first, player, misere, preferansPlayer, trumpChanged)
						CARD_INDEX preferansPlayer[] = { CARD_INDEX_NORTH,
								CARD_INDEX_EAST, CARD_INDEX_SOUTH };
						const CARD_INDEX player = CARD_INDEX_NORTH;		//?
						pr.solveEstimateOnly(ci, trump, first, player, misere,
								preferansPlayer, trumpChanged);
						//pr.m_e
					}
					if (print) {
						printl(br.m_e,br.m_ns, br.m_ew,timeElapse(begin))
						exit(1);
					}

					count++;
					if (count % 500 == 0) {
						double o = timeElapse(begin) / count
								* totalPositions(bridge) / 3600.
								/ getNumberOfCores();
						println("%s %d %.2lf, avgFull %.2lf (hours)",
								trump == NT ? "NT" : "trump", count,
								timeElapse(begin), o)
						fflush(stdout);
//						file.close();
//						return;
					}
				}
			}
		}
		file.close();
		break;//TODO
	}

	println("time %.2lf", timeElapse(begin))
}


}

#endif /* ENDGAME_H_ */
