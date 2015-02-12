#include "DiskItemReader.hpp"
#include "DiskBufferReader.hpp"

using namespace std;

unsigned DiskItemReader::getItemSize()
{
	return buffer->itemSize;
}

DiskItemReader::~DiskItemReader()
{
	if(buffer!=NULL) buffer->freeUsage();
}
