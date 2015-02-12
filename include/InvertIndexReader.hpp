#ifndef GOUGOU_INVERT_INDEX_READER_
#define GOUGOU_INVERT_INDEX_READER_

#include <stdint.h>
#include <vector>

#include "DAMreader.hpp"
#include "PForDecompressor.hpp"
#include "TermSummaryForInvert.hpp"
#include "Posting.hpp"

using namespace std;

class PostingListReader {
public:
  PostingListReader(const TermSummaryForInvert& term, char* data, PForDecompressor *decompressor, unsigned termID);
  inline bool NextBlock() { return JumpToBlock(++cur_block_); }
  inline bool NextDoc() {
    if (cur_pos_ + 1 < cur_block_length_) {
    cur_pos_++;
    return true;
    }
    return NextBlock();
  }
  inline unsigned CurDocID() const { return docID_[cur_pos_]; }
  inline unsigned CurTF() const { return TF_[cur_pos_]; }
  Posting CurPosting() const;
  inline unsigned GetTermID() const { return termID_; }
  inline const TermSummaryForInvert& GetSummary() const { return term_summary_;}
  bool JumpToBlock(unsigned n);
  ~PostingListReader();
private:
  TermSummaryForInvert term_summary_;
  vector<unsigned> block_address_;
  unsigned docID_[COMPRESS_BLOCK_SIZE*2];
  unsigned TF_[COMPRESS_BLOCK_SIZE*2];
  int cur_block_;
  unsigned cur_block_length_;
  unsigned cur_pos_;
  unsigned termID_;
  PForDecompressor* decompressor_;
  char* data_;
};

///////////////////////////////////////////////////////////////

class InvertIndexReader {
public:
  InvertIndexReader(char* term_path,char* invert_path);
  PostingListReader* GetPosting(unsigned n);
  inline unsigned GetTermNum() { return term_num_; }
  ~InvertIndexReader();
private:
  unsigned term_num_;
  DiskAsMemoryReader* term_DAM_;
  DiskAsMemoryReader* invert_DAM_;
  PForDecompressor* decompressor_;
};


#endif
