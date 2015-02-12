#include "TermMeta.hpp"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <vector>

#define TEST_SIZE 1000000
#define TERM_SIZE 1000

#include "Posting.hpp"
#include "PForDecompressor.hpp"

using namespace std;

int main(int argc,char** argv) {
  srand ( unsigned ( std::time(0) ) );
  vector<vector<Posting> > res;
  PForDecompressor* PFD = new PForDecompressor();
  TermMetaManager* TMM = new TermMetaManager("TermMeta_temp");
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
    TermMeta* term_data = TMM->GetTermMeta(i);
    if (res[i].size() != term_data->GetDF()) cout << i << ":" << res[i].size() << " vs " << term_data->GetDF() << endl;
    vector<TermMetaBlock>::const_iterator it;
    unsigned num = 0;
    unsigned docID[COMPRESSOR_BLOCK_SIZE*2] = {};
    unsigned tf[COMPRESSOR_BLOCK_SIZE*2];
    unsigned data1[COMPRESSOR_BLOCK_SIZE*2];
    unsigned data2[COMPRESSOR_BLOCK_SIZE*2];
    for (it = term_data->GetBlock().begin(); it != term_data->GetBlock().end(); it++) {
      unsigned item_num = COMPRESSOR_BLOCK_SIZE;
      if (it+1 == term_data->GetBlock().end()) {
        item_num = term_data->GetDF() % COMPRESSOR_BLOCK_SIZE;
        if (item_num == 0) item_num = COMPRESSOR_BLOCK_SIZE;
      }
      DiskMultiItem* items = TMM->GetDAM()->LocalMultiItem(it->address1,it->length1);
      items -> MemcpyOut((char*)data1,0,it->length1);
      delete items;
      items = TMM->GetDAM()->LocalMultiItem(it->address2,it->length2);
      items -> MemcpyOut((char*)data2,0,it->length2);
      delete items;
      PFD->Decompress(it->a1,it->b1,item_num,data1,it->length1,docID);
      docID[0] += it->docID;
      for (unsigned l = 1; l<item_num; l++) {
        docID[l] += docID[l-1] + 1;
      }
      PFD->Decompress(it->a2,it->b2,item_num,data2,it->length2,tf);
      for (unsigned l = 0; l<item_num; l++) tf[l]++;
      for (unsigned l = 0; l<item_num; l++) {
        if(res[i][num].docID != docID[l]) cout<<"DocID " << i << ":" << num << ":\t" << res[i][num].docID << " vs " << docID[l] <<endl;
        if(res[i][num].tf != tf[l]) cout<<"tf "<< i << ":" << num << ":\t" << res[i][num].tf << " vs " << tf[l] <<endl;
        num++;
      }
    }
  }
  delete TMM;
  delete PFD;
  return 0;
}
