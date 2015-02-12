#ifndef GOUGOU_NAME_LOOKUP_READER_
#define GOUGOU_NAME_LOOKUP_READER_

#include "DAMreader.hpp"

using namespace std;

class NameLookupReader {
public:
  NameLookupReader(char* path);
  string Lookup(unsigned ID);
  inline uint64_t GetNum() { return summary_->getItemCounter(); }
  ~NameLookupReader();
private:
  DiskAsMemoryReader* summary_;
  DiskAsMemoryReader* body_;
};

#endif
