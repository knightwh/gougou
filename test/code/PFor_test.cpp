#include "stdlib.h"
#include <ctime>
#include <iostream>
#include <vector>

#define COMPRESS_BLOCK_SIZE 64
#define TEST_SIZE 200000

#include "PForCompressor.hpp"
#include "PForDecompressor.hpp"

using namespace std;

int main(int argc, char** argv) {
  srand ( unsigned ( std::time(0) ) );
  vector<unsigned> v;
  vector<unsigned> res;
  vector<PForBlock> blocks;
  PForCompressor* PFC = new PForCompressor();
  PForDecompressor* PFD = new PForDecompressor();
  for(unsigned i = 0; i<TEST_SIZE; i++) {
    if(i>0 && i%COMPRESS_BLOCK_SIZE==0) {
      blocks.push_back(PFC->compress(v));
      v.clear();
    }
    unsigned num = rand() % 10000;
    v.push_back(num);
    res.push_back(num);
  }
  if(!v.empty()) blocks.push_back(PFC->compress(v));
  // Begin test the decompressor;
  for(unsigned i = 0; i*COMPRESS_BLOCK_SIZE < TEST_SIZE; i++) {
    unsigned d[COMPRESS_BLOCK_SIZE];
    unsigned length = COMPRESS_BLOCK_SIZE;
    if(length > TEST_SIZE-i*COMPRESS_BLOCK_SIZE) length = TEST_SIZE-i*COMPRESS_BLOCK_SIZE;
    PFD->Decompress(blocks[i].a,blocks[i].b,length,&blocks[i].data[0],blocks[i].data.size(),d);
    for(unsigned l = 0; l<length; l++) {
      if (d[l] != res[i*COMPRESS_BLOCK_SIZE + l])
      cout<<d[l]<<" vs "<<res[i*COMPRESS_BLOCK_SIZE + l]<<endl;
    }
  }
  delete PFC;
  delete PFD;
  return 0;
}
  
