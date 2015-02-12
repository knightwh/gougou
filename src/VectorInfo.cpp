#include "VectorInfo.hpp"

#include <string.h>
#include <sys/stat.h>

using namespace std;

VectorInfo::VectorInfo(char* path) {
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  char summary_path[256];
  char body_path[256];
  strcpy(summary_path,path);
  strcat(summary_path,"/summary");
  strcpy(body_path,path);
  strcat(body_path,"/body");
  compressor_ = new PForCompressor();
  summary_DAM_ = new DiskAsMemory(summary_path,sizeof(VectorInfoUnit),4096);
  body_DAM_ = new DiskAsMemory(body_path,sizeof(unsigned),4096);
}

VectorInfo::~VectorInfo() {
  delete compressor_;
  delete summary_DAM_;
  delete body_DAM_;
}

void VectorInfo::AddVector(const vector<unsigned>& v) {
  AddVector(v, summary_DAM_->getItemCounter());
}

void VectorInfo::AddVector(const vector<unsigned>& v, unsigned ID) {
  DiskItem* item;
  if (ID >= summary_DAM_->getItemCounter()) {
    while (ID > summary_DAM_->getItemCounter()) {
      item = summary_DAM_->addNewItem();
      delete item;
    }
    item = summary_DAM_->addNewItem();
  } else {
    item = summary_DAM_->localItem(ID);
  }
  VectorInfoUnit* u = (VectorInfoUnit*)item->content;
  u->address = body_DAM_->getItemCounter();
  u->length = v.size();
  delete item;
  // write the information into the body.
  unsigned num = 0;
  while (num < v.size()) {
    unsigned length = COMPRESS_BLOCK_SIZE;
    vector<unsigned> data;
    if (length + num > v.size()) length = v.size() - num;
    for (unsigned i=0; i<length; i++) data.push_back(v[num+i]);
    PForBlock block = compressor_->compress(data);
    unsigned temp = block.a & 7;
    temp |= (block.b & 31) << 3;
    temp |= (block.data.size() & 255) << 8;
    temp |= (length & 255) << 16;
    item = body_DAM_->addNewItem();
    memcpy(item->content,&temp,sizeof(unsigned));
    delete item;
    DiskMultiItem* items = body_DAM_->AddMultiNewItem(block.data.size());
    items->MemcpyIn((char*)&block.data[0],0,block.data.size());
    delete items;
    num += length;
  }
}
  
