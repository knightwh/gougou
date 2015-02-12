#ifndef GOUGOU_TERM_META
#define GOUGOU_TERM_META

#include <stdint.h>
#include <vector>
#include "DAM.hpp"
#include "Posting.hpp"
#include "PForCompressor.hpp"

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
  uint64_t address2;
};

class TermMeta {
public:
  TermMeta() {
    tf = 0;
    df = 0;
    cur_docID = 0;
  };
  bool PushPosting(Posting p) {
    if (temp_docID.size() == COMPRESS_BLOCK_SIZE) return false;
    temp_docID.push_back(p.docID);
    temp_tf.push_back(p.tf);
    df++;
    tf+=p.tf;
    return true;
  }
  inline bool TempPostingFull() {return (temp_docID.size() == COMPRESS_BLOCK_SIZE);}
  inline bool TempPostingEmpty() {return temp_docID.empty();}
  inline const vector<TermMetaBlock>& GetBlock() {return blocks;}
  inline vector<unsigned>& GetTempDocID() {return temp_docID;}
  inline vector<unsigned>& GetTempTF() {return temp_tf;}
  inline unsigned GetDF() const {return df;}
  inline unsigned GetTF() const {return tf;}
  void AddBlock(const TermMetaBlock& b) { blocks.push_back(b);}
  void ClearTempPosting() { temp_docID.clear(); temp_tf.clear();}
  inline unsigned GetCurDocID() const { return cur_docID;}
  inline void SetCurDocID(unsigned docID) { cur_docID = docID;}
private:
  vector<TermMetaBlock> blocks;
  vector<unsigned> temp_docID;
  vector<unsigned> temp_tf;
  unsigned cur_docID;
  unsigned df;
  uint64_t tf;
};

class TermMetaManager {
 public:
  TermMetaManager(char* p);
  void PushPosting(unsigned t, Posting p);
  void Finalize();
  vector<pair<unsigned,unsigned> > GetBlockCount() const;
  inline TermMeta* GetTermMeta(unsigned t) {return terms[t];}
  inline unsigned GetTermSize() {return terms.size();}
  inline uint64_t GetTotalTF() {return totalTF;}
  inline DiskAsMemory* GetDAM() {return DAM;}
  ~TermMetaManager();
 protected:
  void AddBlock(unsigned t,const PForBlock& b1,const PForBlock& b2);
  PForBlock CompressDocID(unsigned t);
  PForBlock CompressTF(unsigned t);
 private:
  DiskAsMemory* DAM;
  vector<TermMeta*> terms;
  PForCompressor* compressor;
  uint64_t item_counter;
  uint64_t totalTF;
};
   
  
#endif
