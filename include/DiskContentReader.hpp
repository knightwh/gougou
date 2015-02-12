#ifndef GOUGOU_DISK_CONTENT_READER_
#define GOUGOU_DISK_CONTENT_READER_

#include <stdlib.h>

using namespace std;

class DiskContentReader {
 public:
  DiskContentReader(char* buffer,int offset,unsigned item_size, unsigned item_num);
  inline char* GetCon() { return con_; }
  inline char* GetItem(unsigned n) const {
    if (n>=item_num_) return NULL;
    return con_+item_size_*n;
  }
  inline const char* GetItemUnsafe(unsigned n) const { return con_+item_size_*n;}
  ~DiskContentReader();

 private:
  char* buffer_;
  char* con_;
  unsigned item_size_;
  unsigned item_num_;
  int offset_;
};

#endif
