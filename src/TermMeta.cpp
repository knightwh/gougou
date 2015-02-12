#include "TermMeta.hpp"

#include <vector>
#include "DAM.hpp"
#include "Posting.hpp"
#include "PForCompressor.hpp"

using namespace std;

TermMetaManager::TermMetaManager(char* path) {
  DAM = new DiskAsMemory(path,sizeof(unsigned),4096);
  compressor = new PForCompressor();
  item_counter=0;
  totalTF=0;
}

TermMetaManager::~TermMetaManager() {
  DAM->truncateItem(0);
  delete DAM;
  delete compressor;
  for (unsigned i=0; i<terms.size(); i++) {
    delete terms[i];
  }
}

void TermMetaManager::AddBlock(unsigned t,const PForBlock& b1,const PForBlock& b2) {
  TermMetaBlock block;
  block.a1 = b1.a & 7;
  block.b1 = b1.b & 31;
  block.a2 = b2.a & 7;
  block.b2 = b2.b & 31;
  block.address1 = DAM->getItemCounter();
  block.length1 = b1.data.size() & 255;
  DiskMultiItem* items = DAM->AddMultiNewItem(b1.data.size());
  items->MemcpyIn((char*)&b1.data[0],0,b1.data.size());
  delete items;
  block.address2 = DAM->getItemCounter();
  block.length2 = b2.data.size() & 255;
  block.docID = terms[t]->GetCurDocID();
  items = DAM->AddMultiNewItem(b2.data.size());
  items->MemcpyIn((char*)&b2.data[0],0,b2.data.size());
  delete items;
  terms[t]->AddBlock(block);
}

void TermMetaManager::PushPosting(unsigned t, Posting p) {
  //cout<<"pushing term "<<t<<endl;
  if (t >= terms.size()) {
    while(t >= terms.size()) {
      terms.push_back(new TermMeta());
    }
  }
  if (terms[t]->TempPostingFull()) {
    PForBlock block1 = CompressDocID(t);
    PForBlock block2 = CompressTF(t);
    terms[t]->ClearTempPosting();
    AddBlock(t,block1,block2);
    terms[t]->SetCurDocID(p.docID);
  }
  totalTF += p.tf;
  terms[t]->PushPosting(p);
}
  
void TermMetaManager::Finalize() {
  //cout<<"termSize="<<terms.size()<<endl;
  for (unsigned i=0; i<terms.size(); i++) {
    if (!terms[i]->TempPostingEmpty()) {
      PForBlock block1 = CompressDocID(i);
      PForBlock block2 = CompressTF(i);
      AddBlock(i,block1,block2);
    }
  }
}

vector<pair<unsigned,unsigned> > TermMetaManager::GetBlockCount() const {
  vector<pair<unsigned,unsigned> > res;
  for (unsigned i=0; i<terms.size(); i++) {
    res.push_back(pair<unsigned,unsigned>(i,terms[i]->GetBlock().size()));
  }
  return res;
} 

PForBlock TermMetaManager::CompressDocID(unsigned t) {
  for (unsigned i = terms[t]->GetTempDocID().size()-1; i>=1; i--) {
    terms[t]->GetTempDocID()[i] -= terms[t]->GetTempDocID()[i-1] + 1;
  }
  terms[t]->GetTempDocID()[0] -= terms[t]->GetCurDocID();
  return compressor->compress(terms[t]->GetTempDocID());
}

PForBlock TermMetaManager::CompressTF(unsigned t) {
  for (unsigned i = 0; i<terms[t]->GetTempTF().size(); i++) {
    terms[t]->GetTempTF()[i]--;
  }
  return compressor->compress(terms[t]->GetTempTF());
}
