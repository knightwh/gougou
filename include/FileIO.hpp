#pragma once
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <list>
#include <stdio.h>

using namespace std;

#ifndef MaxFileSize
#define MaxFileSize (1024*1024*1024)
#endif

class FileIO
{
public:
	FileIO(char *foldname,unsigned bN);
	FileIO(char* foldname,unsigned itemSize,unsigned bufferSize,unsigned bufferNum);
	inline char* addNewItem(unsigned &offset);
	inline char* localItem(unsigned offset);
	void deleteItem(unsigned offset);
	unsigned createHeadBuffer(unsigned headBufferSize);
	inline unsigned getItemCounter() {return *itemCounter;}
	inline unsigned getItemSize() {return itemSize;}
	inline unsigned getBufferSize() {return bufferSize;}
	inline unsigned getHeadBufferSize() {return headEnd;}
	bool copyItem(unsigned destination,unsigned source);
	bool switchItem(unsigned itemA,unsigned itemB);
	bool truncateItem(unsigned counter);
	~FileIO();
private:
	char foldPath[256];
	int fileHandle[256];
	int fileHandleSummary;

	unsigned itemSize;
	unsigned bufferPerFile;
	unsigned bufferSpace;
	unsigned bufferSize;
	unsigned bufferNum;
	char *headBuffer,*tailBuffer;
	unsigned tailBegin,headEnd;
	struct bodyBuffer
	{
		char* buffer;
		unsigned begin;
		unsigned end;
		unsigned usage;
	};
	list<bodyBuffer> bodys;
	unsigned bodyBufferN;
	unsigned tailUsage;

	char *metaBuffer;
	unsigned *itemCounter;
	int lastFile;
	unsigned *allocatedSpace;
	
	void expandSize();
	void localTail();
	bool validPara();
};

FileIO::FileIO(char *foldname,unsigned iS,unsigned bS,unsigned bN)
{
	mkdir(foldname,S_IRWXU);
	char filename[512];
	unsigned i;
	itemSize=iS;
	headEnd=0;
	bufferSize=bS;
	bufferNum=bN;
	if(!validPara()) {return;}
	strcpy(foldPath,foldname);
	sprintf(filename,"%s/summary",foldPath);
	fileHandleSummary = open(filename,O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
	if(fileHandleSummary==-1)
	{
		cerr<<"Could not create the new file!"<<endl;
		return;
	}
	lseek(fileHandleSummary,sizeof(unsigned)*2,SEEK_SET);
	write(fileHandleSummary,&itemSize,sizeof(unsigned));
	write(fileHandleSummary,&bufferSize,sizeof(unsigned));
	metaBuffer = (char*)mmap(0,sizeof(unsigned)*4,PROT_READ | PROT_WRITE, MAP_SHARED,fileHandleSummary,0);
	if(metaBuffer == MAP_FAILED)
	{
		close(fileHandleSummary);
		cerr<<"mmap failure at the metafile!"<<endl;
		return;
	}
	itemCounter = (unsigned*)metaBuffer;
	lastFile = -1;
	allocatedSpace = itemCounter+1;
	*itemCounter=0;
	*allocatedSpace=0;
	headBuffer=NULL;
	tailBuffer=NULL;
	for(i=0;i<256;i++) fileHandle[i]=-1;
	expandSize();
}

FileIO::FileIO(char *foldname,unsigned bN)
{
	char filename[512];
	unsigned i;
	headEnd=0;
	//bufferSize=bS;
	bufferNum=bN;
	strcpy(foldPath,foldname);
	sprintf(filename,"%s/summary",foldPath);
	fileHandleSummary = open(filename,O_RDWR);
	if(fileHandleSummary==-1)
	{
		cerr<<"Could not read the old file!"<<endl;
		return;
	}
	metaBuffer = (char*)mmap(0,sizeof(unsigned)*4,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandleSummary,0);
	if(metaBuffer == MAP_FAILED)
	{
		close(fileHandleSummary);
		cerr<<"mmap failure at the metafile!"<<endl;
		return;
	}
	itemCounter = (unsigned*)metaBuffer;
	allocatedSpace = itemCounter+1;
	itemSize = *(itemCounter+2);
	bufferSize = *(itemCounter+3);
	validPara();
	for(i=0;i<256;i++) fileHandle[i]=-1;
	localTail();
}

FileIO::~FileIO()
{
	if(headEnd>0 || headBuffer!=NULL) munmap(headBuffer,headEnd);
	if(tailBuffer!=NULL) munmap(tailBuffer,(*allocatedSpace-tailBegin)*itemSize);
	list<bodyBuffer>::iterator it;
	for(it=bodys.begin();it!=bodys.end();it++)
	{
		munmap(it->buffer,(it->end-it->begin)*itemSize);
	}
	unsigned i;
	for(i=0;i<=lastFile;i++)
	{
		if(fileHandle[i]!=-1) close(fileHandle[i]);
	}
	munmap(metaBuffer,sizeof(unsigned)*4);
	close(fileHandleSummary);
}

bool FileIO::validPara()
{
	bufferSpace = itemSize * bufferSize;
	unsigned pageSize = sysconf(_SC_PAGESIZE);
	if(bufferSpace % pageSize != 0)
	{
		cerr<<"The buffer size should be the integer times page size of "<<pageSize<<endl;
		return false;
	}
	bufferPerFile = MaxFileSize / bufferSpace;
	return true;
}

inline char* FileIO::addNewItem(unsigned &offset)
{
	if(*itemCounter>=*allocatedSpace) expandSize();
	offset = (*itemCounter)++;
	tailUsage++;
	return tailBuffer + (offset - tailBegin)*itemSize;
}

inline char* FileIO::localItem(unsigned offset)
{
	if(offset<headEnd) return headBuffer + offset*itemSize;
	if(offset>*itemCounter) return NULL;
	if(offset>=tailBegin) 
	{
		tailUsage++;
		return tailBuffer + (offset-tailBegin)*itemSize;
	}
	list<bodyBuffer>::iterator it;
	for(it=bodys.begin();it!=bodys.end();it++)
	{
		if(it->begin<=offset && it->end>offset)
		{
			if(it->usage==0)
			{
				it->usage=1;
				bodys.push_back(*it);
				char* theLocal = it->buffer + (offset-it->begin)*itemSize;
				bodys.erase(it);
				return theLocal;
			}
			else 
			{
				it->usage++;
				return it->buffer + (offset-it->begin)*itemSize;
			}
		}
	}
	// no buffer found
	if(bodys.size() >= bufferNum)
	{
		it = bodys.begin();
		if(it->usage==0)
		{
			munmap(it->buffer,(it->end-it->begin)*itemSize);
			bodys.erase(it);
		}
	}
	// no buffer found
	unsigned theBufferNum = offset / bufferSize;
	unsigned theFileNum = theBufferNum / bufferPerFile;
	unsigned theFilePos = theBufferNum % bufferPerFile * bufferSpace;
	bodyBuffer bB;
	if(fileHandle[theFileNum]==-1)
	{
		char filename[256];
		sprintf(filename,"%s/%d",foldPath,theFileNum);
		fileHandle[theFileNum] = open(filename,O_RDWR);
		if(fileHandle[theFileNum]==-1)
		{
			cerr<<"Could not read the old file! "<<theFileNum<<endl;
			return NULL;
		}
	}
	bB.buffer = (char*)mmap(0,bufferSpace,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandle[theFileNum],theFilePos);
	if(bB.buffer == MAP_FAILED)
	{
		cerr<<"Could not mmap the buffer! "<<endl;
		return NULL;
	}
	bB.begin = theBufferNum * bufferSize;
	bB.end = bB.begin + bufferSize;
	bB.usage = 1;
	bodys.push_back(bB);
	return bB.buffer + (offset - bB.begin)*itemSize;
}

bool FileIO::deleteItem(unsigned offset)
{
	if(offset>*itemCounter) return false;
	if(offset<headEnd ) return true;
	if(offset>=tailBegin)
	{
		if(tailUsage>0) {tailUsage--;return true;}
		else return false;
	}
	
	
	
void FileIO::expandSize()
{
	//store the old tail buffer
	list<bodyBuffer>::iterator it;
	if(tailBuffer!=NULL && tailUsage!=0)
	{
		if(bodys.size() >= bufferNum)
		{
			it = bodys.begin();
			if(it->usage==0)
			{
				munmap(it->buffer,(it->end-it->begin)*itemSize);
				bodys.erase(it);
			}
		}
		bodyBuffer bB;
		bB.buffer = tailBuffer;
		bB.begin = tailBegin;
		bB.end = *allocatedSpace;
		bB.usage = tailUsage;
		bodys.push_back(bB);
		//cout<<"the body from "<<bB.begin<<" to "<<bB.end<<endl;
	}
	//expand the tail
	unsigned theFileNum = (*allocatedSpace/bufferSize)/bufferPerFile;
	unsigned theOffset = (*allocatedSpace/bufferSize)%bufferPerFile*bufferSpace;
	if(*allocatedSpace==0 || theFileNum>lastFile)
	{
		lastFile=theFileNum;
		char filename[256];
		sprintf(filename,"%s/%d",foldPath,lastFile);	
		fileHandle[lastFile] = open(filename,O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
		if(fileHandle[lastFile]==-1)
		{
			cerr<<"Could not create the new file "<<lastFile<<endl;
			return;
		}
	}
	lseek(fileHandle[lastFile],theOffset+bufferSpace-1,SEEK_SET);
	write(fileHandle[lastFile],"",1);
	tailBuffer = (char*)mmap(0,bufferSpace,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandle[lastFile],theOffset);
	if(tailBuffer == MAP_FAILED)
	{
		cerr<<"Could not mmap the buffer at the tail!"<<endl;
		return;
	}
	tailBegin=*allocatedSpace;
	tailUsage = 0;
	*allocatedSpace+=bufferSize;
}

void FileIO::localTail()
{
	*allocatedSpace-=bufferSize;
	unsigned theFileNum = (*allocatedSpace/bufferSize)/bufferPerFile;
	unsigned theOffset = (*allocatedSpace/bufferSize)%bufferPerFile*bufferSpace;
	if(fileHandle[theFileNum]==-1)
	{
		char filename[256];
		sprintf(filename,"%s/%d",foldPath,theFileNum);
		fileHandle[theFileNum] = open(filename,O_RDWR);
		if(fileHandle[theFileNum]==-1)
		{
			cerr<<"Could not read the file "<<filename<<endl;
			return;
		}
	}
	tailBuffer = (char*)mmap(0,bufferSpace,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandle[theFileNum],theOffset);
	tailBegin = *allocatedSpace;
	lastFile=theFileNum;
	*allocatedSpace+=bufferSize;
}

unsigned FileIO::createHeadBuffer(unsigned headBufferSize)
{
	if(headBufferSize > *allocatedSpace) headBufferSize=*allocatedSpace;
	if(headBufferSize > bufferPerFile*bufferSize) headBufferSize=bufferPerFile*bufferSize;
	if(headEnd>0 && headBuffer!=NULL)
	{
		munmap(headBuffer,headEnd*itemSize);
	}
	headEnd=0;
	headBuffer = (char*)mmap(0,headBufferSize*itemSize,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandle[0],0);
	if(headBuffer == MAP_FAILED)
	{
		cerr<<"Could not mmap the head buffer!"<<endl;
		return 0;
	}
	headEnd=headBufferSize;
	return headBufferSize;
}

bool FileIO::truncateItem(unsigned counter)
{
	if(counter>=*allocatedSpace) return false;
	if(counter==0)
	{
		while(lastFile>=0)
                {
                        if(fileHandle[lastFile]!=-1) close(fileHandle[lastFile]);
                        char filename[256];
                        sprintf(filename,"%s/%d",foldPath,lastFile);
                        unlink(filename);
                        lastFile--;
                }
		return true;
	}
	unsigned allocated = (counter+bufferSize-1)/bufferSize*bufferSize;
	if(allocated<*allocatedSpace)
	{
		if(headEnd > allocated)
		{
			munmap(headBuffer,headEnd*itemSize);
			headEnd = allocated;
			headBuffer = (char*)mmap(0,headEnd*itemSize,PROT_READ | PROT_WRITE, MAP_SHARED,fileHandle[0],0);
			if(headBuffer == MAP_FAILED)
			{
				cerr<<"Could not mmap the head buffer!"<<endl;
				headEnd = 0;
			}
		}
		list<bodyBuffer>::iterator it=bodys.begin(),it2;
		while(it!=bodys.end())
		{
			it2 = it++;
			if(it2->end > allocated)
			{
				munmap(it2->buffer,(it2->end-it2->begin)*itemSize);
				bodys.erase(it2);
			}
		}
		munmap(tailBuffer,(*allocatedSpace - tailBegin)*itemSize);
		*allocatedSpace = allocated;
		int theLastFile = lastFile;
		unsigned theOffset = (*allocatedSpace/bufferSize)%bufferPerFile*bufferSpace;
		if(theOffset == 0) theOffset=bufferPerFile*bufferSpace;
		localTail();
		while(theLastFile > lastFile)
		{
			if(fileHandle[theLastFile]!=-1) close(fileHandle[theLastFile]);
			char filename[256];
			sprintf(filename,"%s/%d",foldPath,theLastFile);
			unlink(filename);
			theLastFile--;
		}
		ftruncate(fileHandle[lastFile],theOffset);
	}
	*itemCounter = counter;
	return true;
}

bool FileIO::copyItem(unsigned destination,unsigned source)
{
	if(source >= *itemCounter || destination >= *itemCounter) return false;
	char* itemSource = localItem(source);
	char* itemDestination = localItem(destination);
	memmove(itemDestination,itemSource,itemSize);
	return true;
}

bool FileIO::switchItem(unsigned A,unsigned B)
{
	if(A >= *itemCounter || B>= *itemCounter) return false;
	char* itemA = localItem(A);
	char* itemB = localItem(B);
	char* temp = new char(itemSize);
	memmove(temp,itemA,itemSize);
	memmove(itemA,itemB,itemSize);
	memmove(itemB,temp,itemSize);
	delete[] temp;
	return true;
}
