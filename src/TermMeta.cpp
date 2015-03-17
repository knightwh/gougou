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
  } else if (address_pos_ == 0) block_size_ = TERM_META_BLOCK_ADDRESS_BASE_SIZE;
  else {
    block_size_ *= 2;
  }
  if(items_ != NULL) delete items_;
  block_usage_ = 0;
  items_ = block_DAM_->LocalMultiItem(address_[address_pos_], block_size_);
  return true;
}

/////////////////////////////////////////////////////////////////////////

TermMetaManager::TermMetaManager(char* path) {
  char the_path[256];
  strcpy(the_path,path);
  strcat(the_path,"_body");
  DAM_ = new DiskAsMemory(the_path,sizeof(unsigned),4096);
  strcpy(the_path,path);
  strcat(the_path,"_block");
  block_DAM_ = new DiskAsMemory(the_path,sizeof(TermMetaBlock),256);
  strcpy(the_path,path);
  strcat(the_path,"_term");
  meta_DAM_ = new DiskAsMemory(the_path,sizeof(TermMeta),1024);
  compressor_ = new PForCompressor();
  totalTF_=0;
}

TermMetaManager::~TermMetaManager() {
  DAM_->truncateItem(0);
  delete DAM_;
  block_DAM_->truncateItem(0);
  delete block_DAM_;
  meta_DAM_->truncateItem(0);
  delete meta_DAM_;
  delete compressor_;
}

TermMetaBlock TermMetaManager::CombineBlock(const PForBlock& b1,const PForBlock& b2) {
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
  block.length2 = b2.data.size() & 255;
  items = DAM_->AddMultiNewItem(b2.data.size());
  items->MemcpyIn((char*)&b2.data[0],0,b2.data.size());
  delete items;
  return block;
}

void TermMetaManager::PushPosting(unsigned t, Posting p) {
  while (t >= meta_DAM_->getItemCounter()) {
    AddTerm();
  }
  DiskItem* meta_item = meta_DAM_->localItem(t);
  TermMeta* meta = (TermMeta*)meta_item->content;
  PushPosting(meta,p);
  delete meta_item;
  totalTF_ += p.tf;
}

void TermMetaManager::PushPosting(TermMeta* meta, Posting p) {
  if (meta->temp_size >= COMPRESS_BLOCK_SIZE) {
    PForBlock block1 = CompressDocID(meta->temp_docID,meta->cur_docID,meta->temp_size);
    PForBlock block2 = CompressTF(meta->temp_tf,meta->temp_size);
    meta->temp_size = 0;
    // Add the new block into term meta.
    TermMetaBlock new_block = CombineBlock(block1,block2);
    new_block.docID = meta->cur_docID;
    meta->cur_docID = p.docID;
    if (meta->usage < meta->capcity) {
      DiskItem* item = block_DAM_->localItem(meta->usage + meta->blocks[meta->block_size-1]);
      memcpy(item->content, &new_block, sizeof(TermMetaBlock));
      delete item;
      meta->usage++;
    } else {
      if (meta->capcity == 0) meta->capcity = TERM_META_BLOCK_ADDRESS_BASE_SIZE;
      else meta->capcity *= 2;
      meta->blocks[meta->block_size++] = block_DAM_->getItemCounter();
      DiskMultiItem* items = block_DAM_->AddMultiNewItem(meta->capcity);
      items->MemcpyIn((char*)&new_block, 0, 1);
      meta->usage = 1;
      delete items;
    }
  }
  meta->temp_docID[meta->temp_size] = p.docID;
  meta->temp_tf[meta->temp_size] = p.tf;
  meta->temp_size++;
  meta->df++;
  meta->tf+=p.tf; 
  totalTF_ += p.tf;
}

void TermMetaManager::AddTerm() {
  DiskItem* item = meta_DAM_->addNewItem();
  TermMeta* meta = (TermMeta*)item->content;
  meta->block_size = 0;
  meta->temp_size = 0;
  meta->capcity = 0;
  meta->usage = 0;
  meta->cur_docID = 0;
  meta->df = 0;
  meta->tf = 0;
  delete item;
}
  
void TermMetaManager::Finalize() {
  for (unsigned i=0; i<meta_DAM_->getItemCounter(); i++) {
    DiskItem* meta_item = meta_DAM_->localItem(i);
    TermMeta* meta = (TermMeta*)meta_item->content;
    if (meta->temp_size != 0) {
      PForBlock block1 = CompressDocID(meta->temp_docID,meta->cur_docID, meta->temp_size);
      PForBlock block2 = CompressTF(meta->temp_tf,meta->temp_size);
    // Add the new block into term meta.
      TermMetaBlock new_block = CombineBlock(block1,block2);
      new_block.docID = meta->cur_docID;
      if (meta->usage < meta->capcity) {
      DiskItem* item = block_DAM_->localItem(meta->usage + meta->blocks[meta->block_size-1]);
      memcpy(item->content, &new_block, sizeof(TermMetaBlock));
      delete item;
      meta->usage++;
      } else {
        meta->capcity = 1;
        meta->blocks[meta->block_size++] = block_DAM_->getItemCounter();
        DiskItem* item = block_DAM_->addNewItem();
        memcpy(item->content, &new_block, sizeof(TermMetaBlock));
        meta->usage = 1;
        delete item;
      }
    }
    delete meta_item;
  }
}

vector<pair<unsigned,unsigned> > TermMetaManager::GetBlockCount() const {
  vector<pair<unsigned,unsigned> > res;
  for (unsigned i=0; i<meta_DAM_->getItemCounter(); i++) {
    DiskItem* meta_item = meta_DAM_->localItem(i);
    TermMeta* meta = (TermMeta*)meta_item->content;
    unsigned finish_num = (meta->block_size>0)?(meta->block_size-1):0;
    unsigned block_num = ((1<<finish_num)-1)*TERM_META_BLOCK_ADDRESS_BASE_SIZE + meta->usage;
    res.push_back(pair<unsigned,unsigned>(i,block_num));
    delete meta_item;
  }
  return res;
} 

unsigned TermMetaManager::GetBlockNum(unsigned t) const {
  DiskItem* meta_item = meta_DAM_->localItem(t);
  TermMeta* meta = (TermMeta*)meta_item->content;
  unsigned finish_num = (meta->block_size>0)?(meta->block_size-1):0;
  unsigned block_num = ((1<<finish_num)-1)*TERM_META_BLOCK_ADDRESS_BASE_SIZE + meta->usage;
  delete meta_item;
  return block_num;
}

PForBlock TermMetaManager::CompressDocID(unsigned docID[],unsigned cur_docID, unsigned length) {
  vector<unsigned> temp;
  temp.push_back(docID[0] - cur_docID);
  for (unsigned i=1; i<length; i++) {
    temp.push_back(docID[i] - docID[i-1] - 1);
  }
  return compressor_->compress(temp);
}

PForBlock TermMetaManager::CompressTF(unsigned tf[],unsigned length) {
  vector<unsigned> temp;
  for (unsigned i = 0; i<length; i++) {
    temp.push_back(tf[i]-1);
  }
  return compressor_->compress(temp);
}

TermMetaIterator* TermMetaManager::GetBlockIterator(unsigned t) {
  DiskItem* meta_item = meta_DAM_->localItem(t);
  TermMeta* meta = (TermMeta*)meta_item->content;
  vector<uint64_t> blocks;
  for (unsigned i = 0; i<meta->block_size; i++) {
    blocks.push_back(meta->blocks[i]);
  }
  TermMetaIterator* it = new TermMetaIterator(blocks,block_DAM_,meta->usage);
  delete meta_item;
  return it;
}
