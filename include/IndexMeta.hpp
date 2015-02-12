#ifndef GOUGOU_INDEX_META
#define GOUGOU_INDEX_META

#include "DAM.hpp"
#include "TermMeta.hpp"

using namespace std;

class IndexMeta {
 public:
  IndexMeta(char temp_path);
  void PushPosting(Posting p,unsigned t);
  void GenerateInvertIndex(char path);
 private:
  unsigned CompressBlock(vector<Posting> p);
  void AddBlock(t);

  vector<TermMeta> term_meta;
  DiskAsMemory * dam;
  Compresser dfdfs;
};

#endif
  
