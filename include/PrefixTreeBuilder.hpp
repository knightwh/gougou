#ifndef GOUGOU_PREFIX_TREE_BUILDER_
#define GOUGOU_PREFIX_TREE_BUILDER_

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "DAM.hpp"

#define Prefix_Tree_Node_Size 7
#define Prefix_Tree_Fast_Buffer_Size (1024*1024)

using namespace std;

struct PrefixTreeNode {
  unsigned value;
  unsigned char children_num;
  pair<char, uint64_t> children[Prefix_Tree_Node_Size];
  uint64_t next;
};

class PrefixTreeBuilder {
public:
  PrefixTreeBuilder(char* path);
  unsigned LookupTerm(char* term);
  unsigned LookupTerm(string s);
  void InsertTerm(char *term, unsigned v);
  void InsertTerm(string s, unsigned v);
  unsigned InsertTerm(string s);
  unsigned InsertTerm(char* term);
  void WriteToFile(char* filePath);
  ~PrefixTreeBuilder();
protected:
  void InitialNode(DiskItem* item, unsigned v) const;
  void InsertTermToNode(uint64_t address, char *term, unsigned v);
  unsigned LookupTermFromNode(uint64_t address, char *term);
private:
  unsigned vsize_;
  DiskAsMemory *DAM_;
  DiskMultiItem *fast_buffer_;
};

#endif
