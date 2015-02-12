#ifndef GOUGOU_VECTOR_INFO_
#define GOUGOU_VECTOR_INFO_

// This class tries to store the vector infomation. e.g. forward document index.

#include <stdint.h>
#include <vector>
#include "DAM.hpp"
#include "PForCompressor.hpp"

using namespace std;

struct VectorInfoUnit {
  uint64_t address;
  unsigned length;
};

class VectorInfo {
public:
  VectorInfo(char* path);
  void AddVector(const vector<unsigned>& v);
  void AddVector(const vector<unsigned>& v, unsigned ID);
  inline unsigned GetNum() const {return summary_DAM_->getItemCounter();}
  ~VectorInfo();
private:
  PForCompressor* compressor_;
  DiskAsMemory* summary_DAM_;
  DiskAsMemory* body_DAM_;
};


#endif
