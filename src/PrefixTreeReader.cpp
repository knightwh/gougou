#include "PrefixTreeReader.hpp"

#include <stdint.h>

using namespace std;

PrefixTreeReader::PrefixTreeReader(char* path) {
  DAM_ = NULL;
  head_ = NULL;
  DAM_ = new DiskAsMemoryReader(path);
  if(DAM_ == NULL || DAM_->getItemSize() != sizeof(char)) {
    cerr<<"The path of the prefix tree is incorrect!"<<endl;
    return;
  }
  DiskItemReader* DI = DAM_->localItem(0);
  head_length_ = *DI->content;
  delete DI;
  unsigned the_size = sizeof(unsigned)+(sizeof(char)*2+sizeof(uint64_t))*head_length_;
  head_ = DAM_->LocalMultiItem(1,the_size);
}

PrefixTreeReader::~PrefixTreeReader() {
  if(head_ != NULL) delete head_;
  if(DAM_ != NULL) delete DAM_;
}

unsigned PrefixTreeReader::LookupTerm(char* term) {
  if (*term == '\0') return 0;
  return Lookup(term,head_length_,head_);
}

unsigned PrefixTreeReader::LookupTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return LookupTerm(term);
}

unsigned PrefixTreeReader::Lookup(char* term, unsigned length, DiskMultiItemReader* items) {
  char ch = *term;
 if (ch == '\0') {
    unsigned value;
    items->MemcpyOut((char*)&value,0,sizeof(unsigned));
    if (items != head_) delete items;
    return value;
 }
  
  for (unsigned i=0; i<length; i++) {
    unsigned offset = sizeof(unsigned)+(sizeof(char)*2+sizeof(uint64_t))*i;
    char letter;
    items->MemcpyOut(&letter,offset,sizeof(char));
    if(ch==letter) {
      unsigned char child_length;
      items->MemcpyOut((char*)&child_length,offset+sizeof(char),sizeof(char));
      unsigned the_size = sizeof(unsigned)+(sizeof(char)*2+sizeof(uint64_t))*child_length;
      uint64_t address;
      items->MemcpyOut((char*)&address,offset+sizeof(char)*2,sizeof(uint64_t));
      if(items != head_) delete items;
      items = DAM_->LocalMultiItem(address,the_size);
      return Lookup(term+1,(unsigned)child_length,items);
    }
  }
  if (items != head_) delete items;
  return 0;
}

