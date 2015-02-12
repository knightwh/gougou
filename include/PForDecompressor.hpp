#ifndef GOUGOU_PFOR_DECOMPRESSOR_
#define GOUGOU_PFOR_DECOMPRESSOR_

#include <vector>
#include "PForCompressor.hpp"

#ifndef COMPRESSOR_BLOCK_SIZE
#define COMPRESSOR_BLOCK_SIZE 64
#endif

using namespace std;

class PForDecompressor {
public:
  PForDecompressor();
  void Decompress(unsigned a,unsigned b,unsigned item_num,unsigned *data,unsigned data_length,unsigned* res);
private:
  typedef void(PForDecompressor::*step1Funs)();
  typedef void(PForDecompressor::*step2Funs)(unsigned);
  step1Funs step1_[17];
  step2Funs step2_[5];
  
  unsigned item_num_;
  unsigned data_length_;
  unsigned* data_;
  unsigned* res_;

  void step3();
  void step1B1();
  void step1B2();
  void step1B3();
  void step1B4();
  void step1B5();
  void step1B6();
  void step1B8();
  void step1B10();
  void step1B16();
  void step1Ex();

  void step2A1(unsigned b);
  void step2A2(unsigned b);
  void step2A3(unsigned b);
  void step2A4(unsigned b);
};

#endif
