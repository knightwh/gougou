#ifndef GOUGOU_PFOR_COMPRESSOR_
#define GOUGOU_PFOR_COMPRESSOR_

#include <vector>

#ifndef PFOR_THRESHOLD
#define PFOR_THRESHOLD 0.9
#endif

#ifndef COMPRESS_BLOCK_SIZE
#define COMPRESS_BLOCK_SIZE 64
#endif

using namespace std;

struct PForBlock {
  unsigned a,b;
  vector<unsigned> data;
};

class PForCompressor {
 public:
  PForCompressor() {
    bsets_[0]=1;bsets_[1]=2;bsets_[2]=3;bsets_[3]=4;bsets_[4]=5;bsets_[5]=6;
    bsets_[6]=8;bsets_[7]=10;bsets_[8]=16;
  }
  PForBlock compress(const vector<unsigned> &v);
 protected:
  void GetAB(const vector<unsigned> &v);

  unsigned bsets_[9];
  unsigned a_,b_;
};

#endif
