#ifndef GOUGOU_TERM_NAME_LOOKUP_
#define GOUGOU_TERM_NAME_LOOKUP_

#include "DAM.hpp"

using namespace std;

class TermNameLookup {
public:
  TermNameLookup(char* path);
  void AddDoc(char* term_name,unsigned docID);
  void AddDoc(string term_name,unsigned docID);
  string GetDocName(unsigned docID);
  ~TermNameLookup();
private:
  DiskAsMemory* summary_;
  DiskAsMemory* body_;
};

#endif
