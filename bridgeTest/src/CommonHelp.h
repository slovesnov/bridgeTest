/*
 * CommonHelp.h
 *
 *  Created on: 05.12.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef COMMONHELP_H_
#define COMMONHELP_H_

#include <sys/stat.h>//stat

const char SHARED_FILE_NAME[]="shared.txt";

bool fileExists (const char* path) {
  struct stat buffer;
  return (stat (path, &buffer) == 0);
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

int getNextProceedValue(){
	auto f=openSharedFile(SHARED_FILE_NAME,"r+");
	int v;
	fscanf(f,"%d",&v);
	fseek(f,0,SEEK_SET);
	fprintf(f,"%d",v+1);
	fclose(f);
	return v;
}




#endif /* COMMONHELP_H_ */
