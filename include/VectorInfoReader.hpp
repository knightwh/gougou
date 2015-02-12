#ifndef GOUGOU_VECTOR_INFO_READER_
#define GOUGOU_VECTOR_INFO_READER_

#include <stdint.h>
#include <vector>
#include "DAMreader.hpp"
#include "PForDecompressor.hpp"
#include "VectorInfo.hpp"

using namespace std;

class VectorInfoReader {
public:
  VectorInfoReader(char* path);
  inline unsigned GetNum() { return summary_DAM_->getItemCounter();}
  unsigned GetLength(unsigned n);
  vector<unsigned> GetVector(unsigned n);
  ~VectorInfoReader();
private:
  PForDecompressor* decompressor_;
  DiskAsMemoryReader* summary_DAM_;
  DiskAsMemoryReader* body_DAM_;
};

#endif
