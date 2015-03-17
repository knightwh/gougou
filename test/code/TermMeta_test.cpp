#include "TermMeta.hpp"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <vector>

#define TEST_SIZE 100000
#define TERM_SIZE 100

#include "Posting.hpp"
#include "PForDecompressor.hpp"

using namespace std;

int main(int argc,char** argv) {
  srand ( unsigned ( std::time(0) ) );
  vector<vector<Posting> > res;
  PForDecompressor* PFD = new PForDecompressor();
  TermMetaManager* TMM = new TermMetaManager("../../gougou_test/TermMeta_temp");
  cout << "OK" <<endl;
  vector<unsigned> cur_docID;
  for(unsigned i=0; i<TEST_SIZE; i++) {
    unsigned termID = rand() % (res.size() + 1);
    if (termID >= res.size()) {
      res.resize(termID+1);
      cur_docID.resize(termID+1);
      cur_docID[termID] = 0;
    }
    Posting p;
    unsigned delta = rand() % 100+1;
    p.docID = cur_docID[termID] + delta;
    cur_docID[termID] = p.docID;
    p.tf = rand() % 100+1;
    res[termID].push_back(p);
    TMM->PushPosting(termID,p);
  }
  TMM->Finalize();

  // Check the res;
  cout << "Begin test the result" <<endl;
  for (unsigned i=0; i<res.size(); i++) {
    DiskItem* meta_item = TMM->GetTermMeta(i);
    TermMeta* term_data = (TermMeta*)meta_item->content;
    if (res[i].size() != term_data->df) cout << i << ":" << res[i].size() << " vs " << term_data->df << endl;
    TermMetaIterator* it = TMM->GetBlockIterator(i);
    unsigned num = 0;
    unsigned docID[COMPRESSOR_BLOCK_SIZE*2] = {};
    unsigned tf[COMPRESSOR_BLOCK_SIZE*2];
    unsigned data1[COMPRESSOR_BLOCK_SIZE*2];
    unsigned data2[COMPRESSOR_BLOCK_SIZE*2];
    //for (it = term_data->GetBlock().begin(); it != term_data->GetBlock().end(); it++) {
    while(it->Next()) {
      unsigned item_num = COMPRESSOR_BLOCK_SIZE;
      //if (it+1 == term_data->GetBlock().end()) {
      if(!it->HasMore()) {
        item_num = term_data->df % COMPRESSOR_BLOCK_SIZE;
        if (item_num == 0) item_num = COMPRESSOR_BLOCK_SIZE;
      }
      TermMetaBlock block = it->Current();
      DiskMultiItem* items = TMM->GetDAM()->LocalMultiItem(block.address1,block.length1);
      items -> MemcpyOut((char*)data1,0,block.length1);
      delete items;
      items = TMM->GetDAM()->LocalMultiItem(block.address1+block.length1,block.length2);
      items -> MemcpyOut((char*)data2,0,block.length2);
      delete items;
      PFD->Decompress(block.a1,block.b1,item_num,data1,block.length1,docID);
      docID[0] += block.docID;
      for (unsigned l = 1; l<item_num; l++) {
        docID[l] += docID[l-1] + 1;
      }
      PFD->Decompress(block.a2,block.b2,item_num,data2,block.length2,tf);
      for (unsigned l = 0; l<item_num; l++) tf[l]++;
      for (unsigned l = 0; l<item_num; l++) {
        if(res[i][num].docID != docID[l]) cout<<"DocID " << i << ":" << num << ":\t" << res[i][num].docID << " vs " << docID[l] <<endl;
        if(res[i][num].tf != tf[l]) cout<<"tf "<< i << ":" << num << ":\t" << res[i][num].tf << " vs " << tf[l] <<endl;
        num++;
      }
    }
    delete it;
    delete meta_item;
  }
  delete TMM;
  delete PFD;
  return 0;
}
