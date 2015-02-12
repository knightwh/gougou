#pragma once
#include <stdint.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>

#include "DiskItemReader.hpp"
#include "DiskMultiItemReader.hpp"

using namespace std;

class DiskAsMemoryReader;
class DiskMultiItemReader;

class DiskBufferReader
{
public:
	DiskBufferReader(DiskAsMemoryReader *theIO,uint64_t begin,uint64_t end,int fileHandle,int offset);
	DiskItemReader* localItem(uint64_t itemNum);
  bool PutMultiItems(DiskMultiItemReader* DMI,uint64_t begin_num,unsigned item_num);
	inline unsigned getUsage() {return usage;}
	void freeUsage();
	const uint64_t begin;
	const uint64_t end;
	const unsigned itemSize;
	DiskAsMemoryReader* const theIO;
	~DiskBufferReader();
private:	
	char* buffer;
	unsigned usage;
};
