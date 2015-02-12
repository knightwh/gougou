#include "NameLookupReader.hpp"

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;

NameLookupReader::NameLookupReader(char* path) {
  char summary_path[256],body_path[256];
  strcpy(summary_path,path);
  strcpy(body_path,path);
  strcat(summary_path,"/summary");
  strcat(body_path,"/body");
  summary_ = new DiskAsMemoryReader(summary_path);
  body_ = new DiskAsMemoryReader(body_path);
}

string NameLookupReader::Lookup(unsigned ID) {
  if(ID+1 >= summary_->getItemCounter()) {
    string res;
    return res;
  }
  DiskItemReader* item1 = summary_->localItem(ID);
  DiskItemReader* item2 = summary_->localItem(ID+1);
  uint64_t address = *(uint64_t*)item1->content;
  unsigned length = *(uint64_t*)item2->content-*(uint64_t*)item1->content;
  delete item2;
  delete item1;
  DiskMultiItemReader* items = body_->LocalMultiItem(address,length);
  char name[length+1];
  items->MemcpyOut(name,0,length);
  delete items;
  name[length]='\0';
  string res(name);
  return res;
}

NameLookupReader::~NameLookupReader() {
  delete summary_;
  delete body_;
}
  
