#include "PForDecompressor.hpp"

#include <iostream>

using namespace std;

PForDecompressor::PForDecompressor() {
  step1_[0]=&PForDecompressor::step1Ex;
  step1_[1]=&PForDecompressor::step1B1;
  step1_[2]=&PForDecompressor::step1B2;
  step1_[3]=&PForDecompressor::step1B3;
  step1_[4]=&PForDecompressor::step1B4;
  step1_[5]=&PForDecompressor::step1B5;
  step1_[6]=&PForDecompressor::step1B6;
  step1_[7]=&PForDecompressor::step1Ex;
  step1_[8]=&PForDecompressor::step1B8;
  step1_[9]=&PForDecompressor::step1Ex;
  step1_[10]=&PForDecompressor::step1B10;
  step1_[11]=&PForDecompressor::step1Ex;
  step1_[12]=&PForDecompressor::step1Ex;
  step1_[13]=&PForDecompressor::step1Ex;
  step1_[14]=&PForDecompressor::step1Ex;
  step1_[15]=&PForDecompressor::step1Ex;
  step1_[16]=&PForDecompressor::step1B16;
  step2_[1]=&PForDecompressor::step2A1;
  step2_[2]=&PForDecompressor::step2A2;
  step2_[3]=&PForDecompressor::step2A3;
  step2_[4]=&PForDecompressor::step2A4;
}

void PForDecompressor::Decompress(unsigned a,unsigned b,unsigned item_num,unsigned *data,unsigned data_length,unsigned* res) {
  item_num_ = item_num;
  data_length_ = data_length;
  data_ = data;
  res_= res;
  (this->*step1_[b])();
  (this->*step2_[a])(b);
  //step3();
}

void PForDecompressor::step1B1()
{
  unsigned l=(item_num_+31)/32;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 1;
    con[1] = (block>>1) & 1;
    con[2] = (block>>2) & 1;
    con[3] = (block>>3) & 1;
    con[4] = (block>>4) & 1;
    con[5] = (block>>5) & 1;
    con[6] = (block>>6) & 1;
    con[7] = (block>>7) & 1;
    con[8] = (block>>8) & 1;
    con[9] = (block>>9) & 1;
    con[10] = (block>>10) & 1;
    con[11] = (block>>11) & 1;
    con[12] = (block>>12) & 1;
    con[13] = (block>>13) & 1;
    con[14] = (block>>14) & 1;
    con[15] = (block>>15) & 1;
    con[16] = (block>>16) & 1;
    con[17] = (block>>17) & 1;
    con[18] = (block>>18) & 1;
    con[19] = (block>>19) & 1;
    con[20] = (block>>20) & 1;
    con[21] = (block>>21) & 1;
    con[22] = (block>>22) & 1;
    con[23] = (block>>23) & 1;
    con[24] = (block>>24) & 1;
    con[25] = (block>>25) & 1;
    con[26] = (block>>26) & 1;
    con[27] = (block>>27) & 1;
    con[28] = (block>>28) & 1;
    con[29] = (block>>29) & 1;
    con[30] = (block>>30) & 1;
    con[31] = (block>>31) & 1;
    con+=32;
  }
}

void PForDecompressor::step1B2()
{
  unsigned l=(item_num_+15)/16;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 3;
    con[1] = (block>>2) & 3;
    con[2] = (block>>4) & 3;
    con[3] = (block>>6) & 3;
    con[4] = (block>>8) & 3;
    con[5] = (block>>10) & 3;
    con[6] = (block>>12) & 3;
    con[7] = (block>>14) & 3;
    con[8] = (block>>16) & 3;
    con[9] = (block>>18) & 3;
    con[10] = (block>>20) & 3;
    con[11] = (block>>22) & 3;
    con[12] = (block>>24) & 3;
    con[13] = (block>>26) & 3;
    con[14] = (block>>28) & 3;
    con[15] = (block>>30) & 3;
    con+=16;
  }
}

void PForDecompressor::step1B3()
{
  unsigned l=(item_num_+9)/10;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 7;
    con[1] = (block>>3) & 7;
    con[2] = (block>>6) & 7;
    con[3] = (block>>9) & 7;
    con[4] = (block>>12) & 7;
    con[5] = (block>>15) & 7;
    con[6] = (block>>18) & 7;
    con[7] = (block>>21) & 7;
    con[8] = (block>>24) & 7;
    con[9] = (block>>27) & 7;
    con+=10;
  }
}

void PForDecompressor::step1B4()
{
  unsigned l=(item_num_+7)/8;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 15;
    con[1] = (block>>4) & 15;
    con[2] = (block>>8) & 15;
    con[3] = (block>>12) & 15;
    con[4] = (block>>16) & 15;
    con[5] = (block>>20) & 15;
    con[6] = (block>>24) & 15;
    con[7] = (block>>28) & 15;
    con+=8;
  }
}

void PForDecompressor::step1B5()
{
  unsigned l=(item_num_+5)/6;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 31;
    con[1] = (block>>5) & 31;
    con[2] = (block>>10) & 31;
    con[3] = (block>>15) & 31;
    con[4] = (block>>20) & 31;
    con[5] = (block>>25) & 31;
    con+=6;
  }
}

void PForDecompressor::step1B6()
{
  unsigned l=(item_num_+4)/5;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 63;
    con[1] = (block>>6) & 63;
    con[2] = (block>>12) & 63;
    con[3] = (block>>18) & 63;
    con[4] = (block>>24) & 63;
    con+=5;
  }
}

void PForDecompressor::step1B8()
{
  unsigned l=(item_num_+3)/4;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 255;
    con[1] = (block>>8) & 255;
    con[2] = (block>>16) & 255;
    con[3] = (block>>24) & 255;
    con+=4;
  }
}

void PForDecompressor::step1B10()
{
  unsigned l=(item_num_+2)/3;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & 1023;
    con[1] = (block>>10) & 1023;
    con[2] = (block>>20) & 1023;
    con+=3;
  }
}

void PForDecompressor::step1B16()
{
  unsigned l=(item_num_+1)/2;
  unsigned i,block;
  unsigned *con=res_;
  for(i=0;i<l;i++)
  {
    block=*(data_++);
    data_length_--;
    con[0] = block & ((1<<16)-1);
    con[1] = block>>16;
    con+=2;
  }
}

void PForDecompressor::step1Ex()
{
  cerr<<"Invalid b value"<<endl;
}

void PForDecompressor::step2A1(unsigned b)
{
  unsigned block;
  while(data_length_ > 0)
  {
    block=*(data_++);
    data_length_--;
    res_[block & 255]+=((block>>8) & 255)<<b;
    res_[(block>>16) & 255]+=((block>>24))<<b;
  }
}

void PForDecompressor::step2A2(unsigned b)
{
  unsigned block1,block2;
  while(data_length_ > 0)
  {
    block1 = *(data_++);
    data_length_--;
    res_[block1 & 255]+=((block1>>8) & 65535)<<b;
    if(data_length_ == 0) break;
    block2 = *(data_++);
    data_length_--;
    res_[block1>>24]+=(block2 & 65535)<<b;
    if(data_length_ == 0) break;
    block1 = *(data_++);
    data_length_--;
    res_[(block2>>16) & 255]+=((block2>>24) + ((block1 & 255)<<8))<<b;
    res_[(block1>>8) & 255]+=(block1>>16)<<b;
  }
}

void PForDecompressor::step2A3(unsigned b)
{
  unsigned block;
  while(data_length_ > 0)
  {
    block= *(data_++);
    data_length_--;
    res_[block & 255]+=(block>>8)<<b;
  }
}

void PForDecompressor::step2A4(unsigned b)
{
  unsigned block1,block2;
  while(true)
  {
    if(data_length_<=1) break;
    block1 = *(data_++);
    block2 = *(data_++);
    data_length_ -= 2;
    res_[block1 & 255]+=( (block1>>8) + ((block2 & 255)<<24) )<<b;
    if(data_length_ == 0) break;
    block1 = *(data_++);
    data_length_--;
    res_[(block2>>8) & 255]+=( (block2>>16) + ((block1 && 65535)<<16) )<<b;
    if(data_length_ == 0) break;
    block2 = *(data_++);
    data_length_--;
    res_[(block1>>16) & 255]+=( (block1>>24) + ((block2 && 16777215)<<8) )<<b;
    if(data_length_ == 0) break;
    block1 = *(data_++);
    data_length_--;
    res_[block2>>24]+=block1;
  }
}

void PForDecompressor::step3() {
  unsigned i;
  for(i=0; i<item_num_;i++) {
    res_[i]++;
  }
}



