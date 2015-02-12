#pragma once

#include <stdint.h>

using namespace std;

class DiskBuffer;

class DiskItem
{
public:
	DiskItem(char* c,uint64_t i,DiskBuffer* b) : content(c),buffer(b),itemNum(i) {};
	unsigned getItemSize();
	~DiskItem();
	char* const content;
	DiskBuffer* const buffer;
	const uint64_t itemNum;
};
