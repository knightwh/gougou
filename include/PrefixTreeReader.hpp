#ifndef GOUGOU_PREFIX_TREE_READER_
#define GOUGOU_PREFIX_TREE_READER_

#include<string.h>
#include<string>

#include "DAMreader.hpp"

using namespace std;

class PrefixTreeReader {
public:
  PrefixTreeReader(char* path);
  unsigned LookupTerm(char* term);
  unsigned LookupTerm(string s);
  ~PrefixTreeReader();
protected:
  unsigned Lookup(char* term, unsigned length, DiskMultiItemReader* items);
private:
  DiskAsMemoryReader* DAM_;
  DiskMultiItemReader* head_;
  unsigned head_length_;
};

#endif
