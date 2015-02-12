#ifndef GOUGOU_TERM_SUMMARY_FOR_INVERT_
#define GOUGOU_TERM_SUMMARY_FOR_INVERT_

#include <stdint.h>

using namespace std;

struct TermSummaryForInvert {
  unsigned df;
  unsigned block_num;
  unsigned data_length;
  uint64_t tf;
  uint64_t address;
};

struct BlockSummaryForInvert {
  unsigned a1 : 3;
  unsigned b1 : 5;
  unsigned length1 : 8;
  unsigned a2 : 3;
  unsigned b2 : 5;
  unsigned length2 : 8;
  unsigned docID;
};

#endif
