#ifndef GOUGOU_TERM_META
#define GOUGOU_TERM_META

#include <stdint.h>
#include <vector>
#include "DAM.hpp"
#include "Posting.hpp"
#include "PForCompressor.hpp"

#define TERM_META_BLOCK_ADDRESS_SIZE 4

using namespace std;

struct TermMetaBlock {
  unsigned a1 : 3;
  unsigned b1 : 5;
  unsigned length1 : 8;
  unsigned a2 : 3;
  unsigned b2 : 5;
  unsigned length2 : 8;
  unsigned docID;
  uint64_t address1;
  //uint64_t address2;
};

class TermMetaIterator {
public:
  TermMetaIterator(const vector<uint64_t>& address, DiskAsMemory* block_DAM, unsigned last_block_size) {
    address_pos_ = -1;
    block_size_ = 0;
    block_usage_ = 0;
    address_ = address;
    block_DAM_ = block_DAM;
    last_block_size_ = last_block_size;
    items_ = NULL;
  }
  inline bool HasMore() {
    if (address_pos_ + 1 < address_.size()) return true;
    return block_usage_ + 1 < block_size_;
  }
  TermMetaBlock Current();
  bool Next();
  ~TermMetaIterator() {
    if (items_ != NULL) delete items_;
  }

private:
  int address_pos_;
  unsigned block_size_;
  unsigned last_block_size_;
  unsigned block_usage_;
  vector<uint64_t> address_;
  DiskAsMemory* block_DAM_;
  DiskMultiItem* items_;
};


class TermMeta {
public:
  TermMeta(DiskAsMemory* block_DAM) {
    tf_ = 0;
    df_ = 0;
    cur_docID_ = 0;
    capcity_ = 0;
    usage_ = 0;
    temp_size_ = 0;
    block_DAM_ = block_DAM;
    items_ = NULL;
  };
  inline bool PushPosting(Posting p) {
    if (temp_size_ >= COMPRESS_BLOCK_SIZE) return false;
    temp_docID_[temp_size_] = p.docID;
    temp_tf_[temp_size_] = p.tf;
    temp_size_++;
    df_++;
    tf_+=p.tf;
    return true;
  }
  inline bool TempPostingFull() const {return (temp_size_ >= COMPRESS_BLOCK_SIZE);}
  inline bool TempPostingEmpty() const {return temp_size_ == 0;}
  //inline const vector<TermMetaBlock>& GetBlock() {return blocks;}
  TermMetaIterator* GetBlockIterator() const {
    return new TermMetaIterator(block_address_, block_DAM_,usage_);
  }
  inline const unsigned* GetTempDocID() const {return temp_docID_;}
  inline const unsigned* GetTempTF() const {return temp_tf_;}
  inline unsigned GetTempSize() const {return temp_size_;}
  inline unsigned GetDF() const {return df_;}
  inline unsigned GetTF() const {return tf_;}
  void AddBlock(const TermMetaBlock& b);
  unsigned GetBlockNum() const;
  void ClearTempPosting() {temp_size_ = 0;}
  inline unsigned GetCurDocID() const { return cur_docID_;}
  inline void SetCurDocID(unsigned docID) { cur_docID_ = docID;}
  ~TermMeta() {
    if(items_ != NULL) delete items_;
  }
private:
  //vector<TermMetaBlock> blocks;
  vector<uint64_t> block_address_;
  unsigned capcity_;
  unsigned usage_;
  unsigned temp_docID_[COMPRESS_BLOCK_SIZE];
  unsigned temp_tf_[COMPRESS_BLOCK_SIZE];
  unsigned temp_size_;
  unsigned cur_docID_;
  unsigned df_;
  uint64_t tf_;
  DiskAsMemory* block_DAM_;
  DiskMultiItem* items_;
};

class TermMetaManager {
 public:
  TermMetaManager(char* p);
  void PushPosting(unsigned t, Posting p);
  void Finalize();
  vector<pair<unsigned,unsigned> > GetBlockCount() const;
  inline TermMeta* GetTermMeta(unsigned t) {return terms_[t];}
  inline unsigned GetTermSize() {return terms_.size();}
  inline uint64_t GetTotalTF() {return totalTF_;}
  inline DiskAsMemory* GetDAM() {return DAM_;}
  ~TermMetaManager();
 protected:
  void AddBlock(unsigned t,const PForBlock& b1,const PForBlock& b2);
  PForBlock CompressDocID(unsigned t);
  PForBlock CompressTF(unsigned t);
 private:
  DiskAsMemory* block_DAM_;
  DiskAsMemory* DAM_;
  vector<TermMeta*> terms_;
  PForCompressor* compressor_;
  uint64_t item_counter_;
  uint64_t totalTF_;
};
   
  
#endif
