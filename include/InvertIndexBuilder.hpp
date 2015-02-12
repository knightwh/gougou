#ifndef GOUGOU_INVERT_INDEX_BUILDER_
#define GOUGOU_INVERT_INDEX_BUILDER_

#include <stdint.h>
#include "DAM.hpp"
#include "TermMeta.hpp"

using namespace std;

class InvertIndexBuilder {
public:
  InvertIndexBuilder(char* temp_path);
  inline void PushPosting(unsigned t, Posting p) {term_meta_->PushPosting(t,p);}
  void Finalize(char* term_path, char* invert_path);
  inline TermMetaManager* GetTermMeta() const {return term_meta_;}
  ~InvertIndexBuilder();
private:
  DiskAsMemory* temp_DAM_;
  TermMetaManager* term_meta_;
};

#endif

