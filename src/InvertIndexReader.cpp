#include "InvertIndexReader.hpp"

#include <stdint.h>
#include <vector>

#include "DAMreader.hpp"
#include "PForDecompressor.hpp"
#include "TermSummaryForInvert.hpp"
#include "Posting.hpp"

using namespace std;

PostingListReader::PostingListReader(const TermSummaryForInvert& term, char* data, PForDecompressor *decompressor, unsigned termID) {
  term_summary_ = term;
  data_ = data;
  decompressor_ = decompressor;
  termID_ = termID;
  
  unsigned summary_size = sizeof(BlockSummaryForInvert)/sizeof(unsigned);
  unsigned address = summary_size * term.block_num;
  block_address_.resize(term.block_num);
  for (unsigned i=0; i<term.block_num; i++) {
    block_address_[i]=address;
    BlockSummaryForInvert* block = ((BlockSummaryForInvert*)data) + i;
    address += block->length1 + block->length2;
  }
  cur_block_ = -1;
  cur_block_length_ = 0;
  cur_pos_ = 0;
}

bool PostingListReader::JumpToBlock(unsigned n) {
  if (n >= term_summary_.block_num) return false;
  cur_block_length_ = COMPRESSOR_BLOCK_SIZE;
  if (n+1 == term_summary_.block_num) {
    cur_block_length_ = term_summary_.df % COMPRESSOR_BLOCK_SIZE;
    if (cur_block_length_ == 0) cur_block_length_ = COMPRESSOR_BLOCK_SIZE;
  }

  BlockSummaryForInvert* block = ((BlockSummaryForInvert*)data_) + n;

  decompressor_->Decompress(block->a1,block->b1,cur_block_length_,(unsigned*)data_ + block_address_[n], block->length1, docID_);
  docID_[0] += block->docID;
  for (unsigned i = 1; i < cur_block_length_; i++) {
    docID_[i] += docID_[i-1] + 1;
  }

  decompressor_->Decompress(block->a2,block->b2, cur_block_length_, (unsigned*)data_ + block_address_[n] + block->length1, block->length2, TF_);

  for (unsigned i = 0; i < cur_block_length_; i++) TF_[i]++;
  
  cur_block_ = n;
  cur_pos_ = 0;
  return true;
}

Posting PostingListReader::CurPosting() const {
  Posting p;
  p.docID = docID_[cur_pos_];
  p.tf = TF_[cur_pos_];
  return p;
}

PostingListReader::~PostingListReader() {
  delete[] data_;
}

/////////////////////////////////////////////////////////////

InvertIndexReader::InvertIndexReader(char* term_path, char* invert_path) {
  term_DAM_ = new DiskAsMemoryReader(term_path);
  invert_DAM_ = new DiskAsMemoryReader(invert_path);
  decompressor_ = new PForDecompressor();
  term_num_ = term_DAM_ -> getItemCounter() - 1;
}

InvertIndexReader::~InvertIndexReader() {
  delete term_DAM_;
  delete invert_DAM_;
  delete decompressor_;
}

PostingListReader* InvertIndexReader::GetPosting(unsigned n) {
  if (n > term_num_ || n == 0) return NULL;
  DiskItemReader* item = term_DAM_->localItem(n);
  TermSummaryForInvert* content = (TermSummaryForInvert*)item->content;
  char* data = invert_DAM_->CopyItems(content->address,content->data_length);
  PostingListReader* p = new PostingListReader(*content, data,decompressor_, n);
  delete item;
  return p;
}
    
    
