#include "PForCompressor.hpp"

#include <algorithm>
#include <vector>

using namespace std;

void PForCompressor::GetAB(const vector<unsigned> &v) {
  vector<unsigned> u=v;
  sort(u.begin(),u.end());
  unsigned threshold = u[((unsigned)(double)u.size()*PFOR_THRESHOLD-1)];
  unsigned max_num = u[u.size()-1];
  unsigned bn=0;
  while((threshold>>bsets_[bn])>0 && bn<8) bn++;
  b_=bsets_[bn];
  max_num>>=b_;
  a_=1;
  while(a_<4 && (max_num >= (1<<(a_*8)))) a_++;
}

PForBlock PForCompressor::compress(const vector<unsigned> &v) {
  GetAB(v);
  unsigned threshold=1<<b_;
  vector<unsigned> tail;
  vector<unsigned>::iterator it;
  PForBlock block;
  
  block.a = a_;
  block.b = b_;
  unsigned head_size = (v.size()+32/b_+1)/(32/b_);
  for(unsigned i=0;i<head_size;i++) block.data.push_back(0);
  for(unsigned i=0;i<v.size();i++) {
    unsigned low=v[i] & (threshold-1);
    block.data[i/(32/b_)] |= low << i%(32/b_)*b_;
    if(v[i] >= threshold) {
      tail.push_back(i);
      unsigned high=v[i]>>b_;
      for(unsigned l=0;l<a_;l++) {
        tail.push_back((high>>(l*8)) & 255);
      }
    }
  }
  unsigned temp=0;
  unsigned i;
  for(i=0; i<tail.size(); i++) {
    temp |= tail[i]<<(i*8%32);
    if(i%4==3) { block.data.push_back(temp);temp=0;}
  }
  if(i%4!=0) block.data.push_back(temp);
  return block;
}
