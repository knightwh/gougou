#pragma once

#include <stdint.h>

using namespace std;

class DiskBufferReader;

class DiskItemReader
{
public:
	DiskItemReader(char* c,uint64_t i,DiskBufferReader* b) : content(c),buffer(b),itemNum(i) {};
	unsigned getItemSize();
	~DiskItemReader();
	char* const content;
	DiskBufferReader* const buffer;
	const uint64_t itemNum;
};
