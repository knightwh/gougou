#ifndef GOUGOU_DISK_MULTI_ITEM_
#define GOUGOU_DISK_MULTI_ITEM_

#include <stdint.h>
#include <vector>

#include "DiskItem.hpp"
#include "DiskBuffer.hpp"

using namespace std;

class DiskMultiItem {
 public:
  DiskMultiItem(uint64_t begin,unsigned item_size);
  DiskItem* GetItem(uint64_t i) const;
  DiskItem* GetItemRel(unsigned i) const;
  bool MemcpyIn(char* source,unsigned offset,unsigned item_num);
  bool MemcpyOut(char* destiny,unsigned offset,unsigned item_num) const;
  bool PutItems(DiskBuffer* DB,char* buffer,unsigned item_num);
  ~DiskMultiItem();
 private:
  struct DiskMultiItemUnit {
    char* buffer;
    unsigned item_num;
    DiskBuffer* DB;
  };
  uint64_t begin_item_;
  unsigned item_num_;
  unsigned item_size_;
  vector<DiskMultiItemUnit> unit_;
};

#endif
