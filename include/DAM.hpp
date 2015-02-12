#pragma once
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <vector>

#include "DiskItem.hpp"
#include "DiskBuffer.hpp"
#include "DiskMultiItem.hpp"

using namespace std;

#ifndef MaxFileSize
#define MaxFileSize (1024*1024*1024)
#endif

class DiskAsMemory
{
public:
	DiskAsMemory(char *foldname);
	DiskAsMemory(char *foldname,unsigned itemSize,unsigned bufferSize);
	DiskItem* addNewItem();
	DiskItem* localItem(uint64_t itemNum);
  DiskMultiItem* AddMultiNewItem(unsigned item_num);
  DiskMultiItem* LocalMultiItem(uint64_t begin_num,unsigned item_num);
	void deleteItem(uint64_t itemNum);
	unsigned createHeadBuffer(unsigned headBufferSize);
	inline uint64_t getItemCounter() const {return *itemCounter;}
	inline unsigned getItemSize() const {return itemSize;}
	inline unsigned getBufferSize() const {return bufferSize;}
	inline unsigned getHeadBufferSize();
	bool truncateItem(uint64_t counter);
	bool freeBuffer(DiskBuffer* DB);
	bool copyItem(uint64_t destination,uint64_t source);
	bool switchItem(uint64_t A,uint64_t B);
	~DiskAsMemory();
private:
	char foldPath[256];
	int fileHandle[256];
	int fileHandleSummary;
	
	unsigned itemSize;
	unsigned bufferPerFile;
	unsigned bufferSpace;
	unsigned bufferSize;
	DiskBuffer* specialHead;
	vector<DiskBuffer*> bodys;
	unsigned tailUsage;

	char* metaBuffer;
	uint64_t *itemCounter;
	int lastFile;
	uint64_t *allocatedSpace;

	void expandSize();
	void localTail();
	bool validPara();
};


	
