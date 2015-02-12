#ifndef GOUGOU_DOC_NAME_LOOKUP_
#define GOUGOU_DOC_NAME_LOOKUP_

#include "DAM.hpp"

using namespace std;

class DocNameLookup {
public:
  DocNameLookup(char* path);
  void AddDoc(char* doc_name,unsigned docID);
  void AddDoc(string doc_name,unsigned docID);
  string GetDocName(unsigned docID);
  inline uint64_t GetDocNum() { return summary_->getItemCounter(); }
  ~DocNameLookup();
private:
  DiskAsMemory* summary_;
  DiskAsMemory* body_;
};

#endif
