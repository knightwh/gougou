#ifndef GOUGOU_DISK_MULTI_ITEM_READER_
#define GOUGOU_DISK_MULTI_ITEM_READER_

#include <stdint.h>
#include <vector>

#include "DiskItemReader.hpp"
#include "DiskBufferReader.hpp"

using namespace std;

class DiskMultiItemReader {
 public:
  DiskMultiItemReader(uint64_t begin,unsigned item_size);
  DiskItemReader* GetItem(uint64_t i) const;
  DiskItemReader* GetItemRel(unsigned i) const;
  bool MemcpyOut(char* destiny,unsigned offset,unsigned item_num) const;
  bool PutItems(DiskBufferReader* DB,char* buffer,unsigned item_num);
  ~DiskMultiItemReader();
 private:
  struct DiskMultiItemUnit {
    char* buffer;
    unsigned item_num;
    DiskBufferReader* DB;
  };
  uint64_t begin_item_;
  unsigned item_num_;
  unsigned item_size_;
  vector<DiskMultiItemUnit> unit_;
};

#endif
