#include "VectorInfoReader.hpp"

#include <string.h>

using namespace std;

VectorInfoReader::VectorInfoReader(char* path) {
  char summary_path[256];
  char body_path[256];
  strcpy(summary_path,path);
  strcat(summary_path,"/summary");
  strcpy(body_path,path);
  strcat(body_path,"/body");
  decompressor_ = new PForDecompressor();
  summary_DAM_ = new DiskAsMemoryReader(summary_path);
  body_DAM_ = new DiskAsMemoryReader(body_path);
}

VectorInfoReader::~VectorInfoReader() {
  delete decompressor_;
  delete summary_DAM_;
  delete body_DAM_;
}

unsigned VectorInfoReader::GetLength(unsigned n) {
  if (n >= summary_DAM_->getItemCounter()) return 0;
  DiskItemReader* item = summary_DAM_->localItem(n);
  unsigned length = ((VectorInfoUnit*)item->content)->length;
  delete item;
  return length;
}

vector<unsigned> VectorInfoReader::GetVector(unsigned n) {
  vector<unsigned> res;
  if (n >= summary_DAM_->getItemCounter()) return res;
  DiskItemReader* item = summary_DAM_->localItem(n);
  unsigned vector_length = ((VectorInfoUnit*)item->content)->length;
  uint64_t address = ((VectorInfoUnit*)item->content)->address;
  delete item;
  while(res.size() < vector_length) {
    item = body_DAM_->localItem(address++);
    unsigned temp = *(unsigned*)item->content;
    delete item;
    unsigned a = temp & 7;
    unsigned b = (temp>>3) & 31;
    unsigned data_length = (temp>>8) & 255;
    unsigned item_num = (temp>>16) & 255;
    unsigned decompress_res[COMPRESS_BLOCK_SIZE*2];
    unsigned data[COMPRESS_BLOCK_SIZE*2];
    DiskMultiItemReader* items = body_DAM_->LocalMultiItem(address,data_length);
    address+=data_length;
    items->MemcpyOut((char*)data,0,data_length);
    delete items;
    decompressor_->Decompress(a,b,item_num,data,data_length,decompress_res);
    for (unsigned i=0; i<item_num; i++) {
      res.push_back(decompress_res[i]);
    }
  }
  return res;
}
