#include "PrefixTree.hpp"

#include <stdint.h>

#include <iostream>
#include <stack>

#include "DAM.hpp"

using namespace std;

unsigned PrefixTreeNode::LookupTerm(char *term) const {
  char ch = *term;
  if (ch == '\0') return value_;
  vector<pair<char, PrefixTreeNode*> >::const_iterator it;
  for (it=children_.begin(); it!=children_.end(); it++) {
    if(it->first == ch) return it->second->LookupTerm(term+1);
  }
  return 0;
}

void PrefixTreeNode::InsertTerm(char* term,unsigned v) {
  char ch = *term;
  if (ch == '\0') {
    value_ = v;
    return;
  }
  vector<pair<char, PrefixTreeNode*> >::iterator it;
  for (it=children_.begin(); it!=children_.end(); it++) {
    if(it->first == ch) {
      it->second->InsertTerm(term+1,v);return;
    }
  }
  PrefixTreeNode *new_node = new PrefixTreeNode();
  children_.push_back(pair<char,PrefixTreeNode*>(ch,new_node));
  new_node->InsertTerm(term+1,v);
}

PrefixTreeNode::~PrefixTreeNode() {
  vector<pair<char, PrefixTreeNode*> >::iterator it;
  for(it=children_.begin(); it!=children_.end(); it++) delete it->second;
}

//---------------------------------------------------------------------//

PrefixTree::PrefixTree() {
  head_ = new PrefixTreeNode();
  vsize_ = 0;
}

unsigned PrefixTree::LookupTerm(char* term) {
  if(*term == '\0') return 0;
  return head_->LookupTerm(term);
}

unsigned PrefixTree::LookupTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return LookupTerm(term);
}

void PrefixTree::InsertTerm(char *term,unsigned v) {
  if(*term == '\0') return;
  head_->InsertTerm(term,v);
  if(vsize_ < v) vsize_ = v;
}

void PrefixTree::InsertTerm(string s,unsigned v) {
  char term[s.length()];
  strcpy(term,s.c_str());
  InsertTerm(term,v);
}

unsigned PrefixTree::InsertTerm(char* term) {
  if(*term == '\0') return 0;
  unsigned termID = head_->LookupTerm(term);
  if (termID == 0) {
    head_->InsertTerm(term,++vsize_);
    return vsize_;
  } else {
    return termID;
  }
}

unsigned PrefixTree::InsertTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return InsertTerm(term);
}

void PrefixTree::WriteToFile(char* filePath) {
  DiskAsMemory *DAM = new DiskAsMemory(filePath,sizeof(char),4096);
  DiskItem* DI = DAM->addNewItem();
  unsigned char head_children_num = head_->GetChildren().size();
  memcpy(DI->content,(char*)&head_children_num,sizeof(char));
  delete DI;
  stack<PrefixTreeNode*> node_stack;
  stack<pair<DiskMultiItem*,unsigned> > parent_stack;
  node_stack.push(head_);

  while(!node_stack.empty()) {
    PrefixTreeNode *cur_node = node_stack.top();
    node_stack.pop();
    unsigned char children_num = cur_node->GetChildren().size();
    unsigned size = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*children_num;
    // Connect it with it parent.
    uint64_t position = DAM->getItemCounter();
    if(!parent_stack.empty()) {
      pair<DiskMultiItem*,unsigned> top_parent = parent_stack.top();
      parent_stack.pop();
      top_parent.second--;
      unsigned offset = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*top_parent.second;
      top_parent.first->MemcpyIn((char*)&position,offset+sizeof(char)*2,sizeof(uint64_t));
      top_parent.first->MemcpyIn((char*)&children_num,offset+sizeof(char),sizeof(char));
      if(top_parent.second > 0) {
        parent_stack.push(top_parent);
      } else {
        delete top_parent.first;
      }
    }

    // Store it into disk.
    DiskMultiItem* items = DAM->AddMultiNewItem(size);
    unsigned value = cur_node->Value();
    items->MemcpyIn((char*)&value,0,sizeof(unsigned));
    if (children_num > 0) {
      parent_stack.push(pair<DiskMultiItem*,unsigned>(items,children_num));
      for(unsigned i=0; i<cur_node->GetChildren().size(); i++) {
        unsigned offset = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*i;
        items->MemcpyIn((char*)&cur_node->GetChildren()[i].first,offset,sizeof(char));
        node_stack.push(cur_node->GetChildren()[i].second);
      }
    } else {
      delete items;
    }
  }
  delete DAM;
}
      
PrefixTree::~PrefixTree() {
   delete head_;
}
    


	
