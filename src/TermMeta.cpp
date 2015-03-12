#include "TermMeta.hpp"

#include <vector>
#include "DAM.hpp"
#include "Posting.hpp"
#include "PForCompressor.hpp"

using namespace std;

TermMetaBlock TermMetaIterator::Current() {
  TermMetaBlock res;
  items_->MemcpyOut((char*)&res,block_usage_,1);
  return res;
}

bool TermMetaIterator::Next() {
  if (block_usage_+1 < block_size_) { block_usage_++; return true; }
  address_pos_++;
  if (address_pos_ >= address_.size()) return false;
  if (address_pos_ + 1 == address_.size()) {
    block_size_ = last_block_size_;
  } else if (address_pos_ == 0) block_size_ = TERM_META_BLOCK_ADDRESS_SIZE;
  else {
    block_size_ *= 2;
  }
  if(items_ != NULL) delete items_;
  block_usage_ = 0;
  items_ = block_DAM_->LocalMultiItem(address_[address_pos_], block_size_);
  return true;
}

/////////////////////////////////////////////////////////////////////////

void TermMeta::AddBlock(const TermMetaBlock& b) {
  if(usage_ < capcity_) {
    items_->MemcpyIn((char*)&b,usage_,1);
    usage_++;
    return;
  } else {
  if (capcity_ == 0) capcity_ = TERM_META_BLOCK_ADDRESS_SIZE;
  else {
    capcity_ *= 2;
    delete items_;
  }
  usage_ = 0;
  block_address_.push_back(block_DAM_->getItemCounter());
  items_ = block_DAM_->AddMultiNewItem(capcity_);
  items_->MemcpyIn((char*)&b,usage_,1);
  usage_++;
  }
}

unsigned TermMeta::GetBlockNum() const {
  return (TERM_META_BLOCK_ADDRESS_SIZE << block_address_.size()) - 
         TERM_META_BLOCK_ADDRESS_SIZE + usage_ - capcity_;
}


///////////////////////////////////////////////////////////////////////

TermMetaManager::TermMetaManager(char* path) {
  char the_path[256];
  strcpy(the_path,path);
  strcat(the_path,"_body");
  DAM_ = new DiskAsMemory(the_path,sizeof(unsigned),4096);
  strcpy(the_path,path);
  strcat(the_path,"_block");
  block_DAM_ = new DiskAsMemory(the_path,sizeof(TermMetaBlock),256);
  compressor_ = new PForCompressor();
  item_counter_=0;
  totalTF_=0;
}

TermMetaManager::~TermMetaManager() {
  for (unsigned i=0; i<terms_.size(); i++) {
    delete terms_[i];
  }
  DAM_->truncateItem(0);
  delete DAM_;
  block_DAM_->truncateItem(0);
  delete block_DAM_;
  delete compressor_;
}

void TermMetaManager::AddBlock(unsigned t,const PForBlock& b1,const PForBlock& b2) {
  TermMetaBlock block;
  block.a1 = b1.a & 7;
  block.b1 = b1.b & 31;
  block.a2 = b2.a & 7;
  block.b2 = b2.b & 31;
  block.address1 = DAM_->getItemCounter();
  block.length1 = b1.data.size() & 255;
  DiskMultiItem* items = DAM_->AddMultiNewItem(b1.data.size());
  items->MemcpyIn((char*)&b1.data[0],0,b1.data.size());
  delete items;
  //block.address2 = DAM_->getItemCounter();
  block.length2 = b2.data.size() & 255;
  block.docID = terms_[t]->GetCurDocID();
  items = DAM_->AddMultiNewItem(b2.data.size());
  items->MemcpyIn((char*)&b2.data[0],0,b2.data.size());
  delete items;
  terms_[t]->AddBlock(block);
}

void TermMetaManager::PushPosting(unsigned t, Posting p) {
  //cout<<"pushing term "<<t<<endl;
  if (t >= terms_.size()) {
    while(t >= terms_.size()) {
      terms_.push_back(new TermMeta(block_DAM_));
    }
  }
  if (terms_[t]->TempPostingFull()) {
    PForBlock block1 = CompressDocID(t);
    PForBlock block2 = CompressTF(t);
    terms_[t]->ClearTempPosting();
    AddBlock(t,block1,block2);
    terms_[t]->SetCurDocID(p.docID);
  }
  totalTF_ += p.tf;
  terms_[t]->PushPosting(p);
}
  
void TermMetaManager::Finalize() {
  //cout<<"termSize="<<terms.size()<<endl;
  for (unsigned i=0; i<terms_.size(); i++) {
    if (!terms_[i]->TempPostingEmpty()) {
      PForBlock block1 = CompressDocID(i);
      PForBlock block2 = CompressTF(i);
      AddBlock(i,block1,block2);
    }
  }
}

vector<pair<unsigned,unsigned> > TermMetaManager::GetBlockCount() const {
  vector<pair<unsigned,unsigned> > res;
  for (unsigned i=0; i<terms_.size(); i++) {
    res.push_back(pair<unsigned,unsigned>(i,terms_[i]->GetBlockNum()));
  }
  return res;
} 

PForBlock TermMetaManager::CompressDocID(unsigned t) {
  vector<unsigned> temp;
  temp.push_back(terms_[t]->GetTempDocID()[0] - terms_[t]->GetCurDocID());
  for (unsigned i=1; i<terms_[t]->GetTempSize(); i++) {
    temp.push_back(terms_[t]->GetTempDocID()[i] - terms_[t]->GetTempDocID()[i-1] - 1);
  }
  return compressor_->compress(temp);
}

PForBlock TermMetaManager::CompressTF(unsigned t) {
  vector<unsigned> temp;
  for (unsigned i = 0; i<terms_[t]->GetTempSize(); i++) {
    temp.push_back(terms_[t]->GetTempTF()[i]-1);
  }
  return compressor_->compress(temp);
}
