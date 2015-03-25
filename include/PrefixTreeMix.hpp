#ifndef GOUGOU_PREFIX_TREE_MIX_
#define GOUGOU_PREFIX_TREE_MIX_

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "DAM.hpp"

#define PREFIX_TREE_NODE_SIZE 7
#define PREFIX_TREE_MEMORY_LEVEL 4

using namespace std;

struct PrefixTreeNodeItem {
  unsigned value;
  unsigned char children_num;
  pair<char, uint64_t> children[PREFIX_TREE_NODE_SIZE];
  uint64_t next;
};

struct PrefixTreeNodeMem;
union PrefixAddress {
  uint64_t disk;
  PrefixTreeNodeMem* pointer;
};

struct PrefixTreeNodeMem {
  unsigned value;
  vector<pair<char, union PrefixAddress> > children;
};

class PrefixTreeMix {
public:
  PrefixTreeMix(char* path);
  unsigned LookupTerm(char* term);
  unsigned LookupTerm(string s);
  void InsertTerm(char *term, unsigned v);
  void InsertTerm(string s, unsigned v);
  unsigned InsertTerm(string s);
  unsigned InsertTerm(char* term);
  void WriteToFile(char* filePath);
  ~PrefixTreeMix();
protected:
  void InitialNode(DiskItem* item, unsigned v) const;
  void InsertTermToNode(uint64_t address, char *term, unsigned v);
  void InsertTermToNode(PrefixTreeNodeMem* node, char *term, unsigned v, unsigned level);
  unsigned LookupTermFromNode(uint64_t address, char *term);
  unsigned LookupTermFromNode(PrefixTreeNodeMem* node, char *term, unsigned level);
private:
  unsigned vsize_;
  DiskAsMemory *DAM_;
  PrefixTreeNodeMem* head_;
};

#endif
