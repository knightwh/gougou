#pragma once
#include <stdint.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>

#include "DiskItem.hpp"
#include "DiskMultiItem.hpp"

using namespace std;

class DiskAsMemory;
class DiskMultiItem;

class DiskBuffer
{
public:
	DiskBuffer(DiskAsMemory *theIO,uint64_t begin,uint64_t end,int fileHandle,int offset);
	DiskItem* localItem(uint64_t itemNum);
  bool PutMultiItems(DiskMultiItem* DMI,uint64_t begin_num,unsigned item_num);
	inline unsigned getUsage() {return usage;}
	void freeUsage();
	//friend class DiskAsMemory;
	const uint64_t begin;
	const uint64_t end;
	const unsigned itemSize;
	DiskAsMemory* const theIO;
	~DiskBuffer();
private:	
	char* buffer;
	unsigned usage;
};
