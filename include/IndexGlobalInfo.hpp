#ifndef GOUGOU_INDEX_GLOBAL_INFO_
#define GOUGOU_INDEX_GLOBAL_INFO_

#include <stdint.h>

using namespace std;

struct IndexGlobalInfo {
  unsigned docN;
  unsigned termN;
  uint64_t totalTF;
};

#endif
