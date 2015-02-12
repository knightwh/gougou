#include "stdlib.h"
#include <ctime>
#include <iostream>

#include "DAM.hpp"
#include "DiskMultiItem.hpp"
#include "DAMreader.hpp"
#include "DiskMultiItemReader.hpp"
#include "DiskContentReader.hpp"

#define TEST_NUM_SIZE (1024*1024*512)

using namespace std;

int main(int argc,char** argv) {
  srand ( unsigned ( std::time(0) ) );
  DiskAsMemory *DAM = new DiskAsMemory("multiItem_temp",sizeof(unsigned),4096);
  unsigned n=0;
  while(n < TEST_NUM_SIZE) {
    unsigned length = rand()%10000+1;
    DiskMultiItem* multi_item = DAM->AddMultiNewItem(length);
    unsigned data[length];
    for(unsigned i=0;i<length;i++) {
      data[i]=n++;
    }
    multi_item->MemcpyIn((char*)data,0,length);
    delete multi_item;
  }

  cout<<"size: "<<n<<" vs "<<DAM->getItemCounter()<<endl;

  // Test the reading.
  for (unsigned i=0; i<10; i++) {
    unsigned num = rand()%(n-20);
    DiskMultiItem* multi_item = DAM->LocalMultiItem(num,10);
    unsigned data[10];
    multi_item->MemcpyOut((char*)data,0,10);
    for (unsigned l=0; l<10; l++) {
      cout<<data[l]<< " vs " <<num+l<<endl;
    }
    delete multi_item;
  }
  delete DAM;

  cout<<"Begin test the reader\n\n\n\n\n"<<endl;
  // Test the reader.
  DiskAsMemoryReader *DAMreader = new DiskAsMemoryReader("multiItem_temp");
  for (unsigned i=0; i<10; i++) {
    unsigned num = rand()%(n-20);
    unsigned length = rand()%20+1;
    DiskMultiItemReader* multi_item = DAMreader->LocalMultiItem(num,length);
    unsigned data[length];
    multi_item->MemcpyOut((char*)data,0,length);
    for (unsigned l=0; l<length; l++) {
      cout<<data[l]<<" vs "<<num+l<<endl;
    }
    delete multi_item;
  }

  cout<<"begin test the content reader\n\n\n\n"<<endl;
  // Test the content reader.
  for (unsigned i=0; i<10; i++) {
    unsigned num = rand()%(n-20);
    unsigned length = rand()%20+1;
    DiskContentReader* DCR = DAMreader->LocalItemContent(num,length);
    for (unsigned l=0; l<length; l++) {
      cout<<*(unsigned*)DCR->GetItem(l)<<" vs "<<num+l<<endl;
    }
    delete DCR;
  }

  cout<<"begin test the item copy\n\n\n\n"<<endl;
  // Test the copy items;
  for (unsigned i=0; i<10; i++) {
    unsigned num = rand()%(n-20);
    unsigned length = rand()%20+1;
    char* record = DAMreader->CopyItems(num,length);
    for (unsigned l=0; l<length; l++) {
      cout<<*(unsigned*)(record+l*sizeof(unsigned))<<" vs "<<num+l<<endl;
    }
    delete[] record;
  }

  delete DAMreader;
  return 0;
}
