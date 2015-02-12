#ifndef GOUGOU_NAME_LOOKUP_
#define GOUGOU_NAME_LOOKUP_

#include "DAM.hpp"

using namespace std;

class NameLookup {
public:
  NameLookup(char* path);
  void AddItem(char* name,unsigned ID);
  void AddItem(string name,unsigned ID);
  string GetName(unsigned ID);
  inline uint64_t GetNum() { return summary_->getItemCounter()-1; }
  ~NameLookup();
private:
  DiskAsMemory* summary_;
  DiskAsMemory* body_;
};

#endif
