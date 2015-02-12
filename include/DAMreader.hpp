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

#include "DiskItemReader.hpp"
#include "DiskBufferReader.hpp"
#include "DiskMultiItemReader.hpp"
#include "DiskContentReader.hpp"

using namespace std;

#ifndef MaxFileSize
#define MaxFileSize (1024*1024*1024)
#endif

class DiskAsMemoryReader
{
public:
	DiskAsMemoryReader(char *foldname);
	DiskItemReader* localItem(uint64_t itemNum);
  DiskMultiItemReader* LocalMultiItem(uint64_t begin_num,unsigned item_num);
  DiskContentReader* LocalItemContent(uint64_t begin_num,unsigned item_num);
  char* CopyItems(uint64_t begin_num,unsigned item_num);
	unsigned createHeadBuffer(unsigned headBufferSize);
	inline uint64_t getItemCounter() const {return *itemCounter;}
	inline unsigned getItemSize() const {return itemSize;}
	inline unsigned getBufferSize() const {return bufferSize;}
	inline unsigned getHeadBufferSize();
	bool freeBuffer(DiskBufferReader* DB);
	~DiskAsMemoryReader();
private:
	char foldPath[256];
	int fileHandle[256];
	int fileHandleSummary;
	
	unsigned itemSize;
	unsigned bufferPerFile;
	unsigned bufferSpace;
	unsigned bufferSize;
	DiskBufferReader* specialHead;
	vector<DiskBufferReader*> bodys;
	unsigned tailUsage;

	char* metaBuffer;
	uint64_t *itemCounter;
	int lastFile;
	uint64_t *allocatedSpace;

	bool validPara();
};


	
