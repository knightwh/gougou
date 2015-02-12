#include "DiskItem.hpp"
#include "DiskBuffer.hpp"

using namespace std;

unsigned DiskItem::getItemSize()
{
	return buffer->itemSize;
}

DiskItem::~DiskItem()
{
  if (buffer != NULL) buffer->freeUsage();
}
