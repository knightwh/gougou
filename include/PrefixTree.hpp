#ifndef GOUGOU_PREFIX_TREE_
#define GOUGOU_PREFIX_TREE_

#include <string.h>
#include <vector>
#include <string>

#include "DAM.hpp"

using namespace std;

class PrefixTreeNode {
public:
  PrefixTreeNode(unsigned v) { value_ = v;}
  PrefixTreeNode() { value_ = 0;}
  unsigned Value() const {return value_;}
  void SetValue(unsigned v) {value_=v;}
  const vector<pair<char, PrefixTreeNode*> >& GetChildren() const {return children_;}
  unsigned LookupTerm(char *term) const;
  void InsertTerm(char *term,unsigned v);
  ~PrefixTreeNode();

private:
  unsigned value_;
  vector<pair<char,PrefixTreeNode*> > children_;
};

//--------------------------------------------------------------//

class PrefixTree {
public:
  PrefixTree();
  unsigned LookupTerm(char *term);
  unsigned LookupTerm(string s);
  void InsertTerm(char *term,unsigned v);
  void InsertTerm(string s,unsigned v);
  unsigned InsertTerm(string s);
  unsigned InsertTerm(char* term);
  void WriteToFile(char* filePath);
  ~PrefixTree();
private:
  unsigned vsize_;
  PrefixTreeNode* head_;
};
    

#endif
