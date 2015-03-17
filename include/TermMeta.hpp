#ifndef GOUGOU_TERM_META
#define GOUGOU_TERM_META

#include <stdint.h>
#include <vector>
#include "DAM.hpp"
#include "Posting.hpp"
#include "PForCompressor.hpp"

#define TERM_META_BLOCK_ADDRESS_BASE_SIZE 4
#define TERM_META_BLOCK_META_SIZE 20

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
};

struct TermMeta {
  unsigned char block_size;
  unsigned char temp_size;
  unsigned capcity;
  unsigned usage;
  unsigned cur_docID;
  unsigned df;
  uint64_t tf;
  uint64_t blocks[TERM_META_BLOCK_META_SIZE];
  unsigned temp_docID[COMPRESS_BLOCK_SIZE];
  unsigned temp_tf[COMPRESS_BLOCK_SIZE];
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

class TermMetaManager {
 public:
  TermMetaManager(char* p);
  void PushPosting(unsigned t, Posting p);
  void Finalize();
  vector<pair<unsigned,unsigned> > GetBlockCount() const;
  unsigned GetBlockNum(unsigned i) const;
  inline DiskItem* GetTermMeta(unsigned t) {
    return meta_DAM_->localItem(t);
  }
  inline unsigned GetTermSize() {return meta_DAM_->getItemCounter();}
  inline uint64_t GetTotalTF() {return totalTF_;}
  inline DiskAsMemory* GetDAM() const { return DAM_;}
  TermMetaIterator* GetBlockIterator(unsigned t);
  ~TermMetaManager();
 protected:
  TermMetaBlock CombineBlock(const PForBlock& b1,const PForBlock& b2);
  void AddTerm();
  PForBlock CompressDocID(unsigned doc[], unsigned cur_docID, unsigned length);
  PForBlock CompressTF(unsigned tf[],unsigned length);
  void PushPosting(TermMeta* meta, Posting p);
 private:
  DiskAsMemory* meta_DAM_;
  DiskAsMemory* block_DAM_;
  DiskAsMemory* DAM_;
  PForCompressor* compressor_;
  uint64_t totalTF_;
};
  
#endif
