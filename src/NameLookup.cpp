#include "NameLookup.hpp"

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;

NameLookup::NameLookup(char* path) {
  mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  char summary_path[256],body_path[256];
  strcpy(summary_path,path);
  strcpy(body_path,path);
  strcat(summary_path,"/summary");
  strcat(body_path,"/body");
  summary_ = new DiskAsMemory(summary_path,sizeof(uint64_t),1024);
  body_ = new DiskAsMemory(body_path,sizeof(char),4096);
  DiskItem* DI=summary_->addNewItem();
  delete DI;
}

void NameLookup::AddItem(char* name,unsigned ID) {
  unsigned length = strlen(name);
  if(ID < summary_->getItemCounter()) {
    cerr<<"Error: Backard define at NameLookup!"<<endl;
    return;
  }
  if(ID > summary_->getItemCounter()) {
    cerr<<"Warning: Jump define at NameLookup!"<<endl;
    while(summary_->getItemCounter() < ID) {
      DiskItem* DI = summary_->addNewItem();
      uint64_t body_usage = body_->getItemCounter();
      memcpy(DI->content,&body_usage,sizeof(uint64_t));
      delete DI;
    }
  }
  DiskItem* DI = summary_->addNewItem();
  uint64_t body_usage = body_->getItemCounter();
  memcpy(DI->content,&body_usage,sizeof(uint64_t));
  delete DI;
  DiskMultiItem* items = body_->AddMultiNewItem(length);
  items->MemcpyIn(name,0,length);
  delete items;
}

void NameLookup::AddItem(string s,unsigned ID) {
  char name[s.length()+1];
  strcpy(name,s.c_str());
  AddItem(name,ID);
}

NameLookup::~NameLookup() {
  DiskItem* DI = summary_->addNewItem();
  uint64_t body_usage = body_->getItemCounter();
  memcpy(DI->content,&body_usage,sizeof(uint64_t));
  delete DI;
  delete summary_;
  delete body_;
}
