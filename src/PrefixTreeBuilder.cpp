#include "PrefixTreeBuilder.hpp"

#include<stack>

using namespace std;

PrefixTreeBuilder::PrefixTreeBuilder(char* path) {
  DAM_ = new DiskAsMemory(path,sizeof(PrefixTreeNode),1024*1024*8);
  fast_buffer_ = NULL;
  DiskItem* item = DAM_->addNewItem();
  InitialNode(item,0);
  delete item;
  vsize_ = 0;
}

PrefixTreeBuilder::~PrefixTreeBuilder() {
  if(fast_buffer_ != NULL) delete fast_buffer_;
  DAM_->truncateItem(0);
  delete DAM_;
}

void PrefixTreeBuilder::InitialNode(DiskItem* item, unsigned v) const {
  PrefixTreeNode* content = (PrefixTreeNode*)item->content;
  content->value = v;
  content->children_num = 0;
  content->next = 0;
}

unsigned PrefixTreeBuilder::LookupTerm(char* term) {
  if(*term == '\0') return 0;
  return LookupTermFromNode(0,term);
}

unsigned PrefixTreeBuilder::LookupTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return LookupTerm(term);
}

void PrefixTreeBuilder::InsertTerm(char *term, unsigned v) {
  if(*term == '\0') return;
  InsertTermToNode(0,term,v);
  if(vsize_ < v) vsize_ = v;
}

void PrefixTreeBuilder::InsertTerm(string s,unsigned v) {
  char term[s.length()];
  strcpy(term,s.c_str());
  InsertTerm(term,v);
}

unsigned PrefixTreeBuilder::InsertTerm(char* term) {
  if(*term == '\0') return 0;
  unsigned termID = LookupTermFromNode(0, term);
  if (termID == 0) {
    InsertTermToNode(0,term,++vsize_);
    return vsize_;
  } else {
    return termID;
  }
}

unsigned PrefixTreeBuilder::InsertTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return InsertTerm(term);
}

unsigned PrefixTreeBuilder::LookupTermFromNode(uint64_t address, char *term) {
  if (address >= DAM_->getItemCounter()) return 0;
  DiskItem* DI;
  if (fast_buffer_ != NULL && address < Prefix_Tree_Fast_Buffer_Size) {
    DI = fast_buffer_->GetItem(address);
  } else {
    DI = DAM_->localItem(address);
  }
  PrefixTreeNode* node = (PrefixTreeNode*)DI->content;
  char ch = *term;
  if (ch == '\0') {
    unsigned value = node->value;
    delete DI;
    return value;
  }
  for(unsigned i=0; i<node->children_num; i++) {
    if(node->children[i].first == ch) {
      uint64_t new_address = node->children[i].second;
      delete DI;
      return LookupTermFromNode(new_address, term+1);
    }
  }
  if (node->next > 0) {
    uint64_t new_address = node->next;
    delete DI;
    return LookupTermFromNode(new_address, term);
  }
  delete DI;
  return 0;
}

void PrefixTreeBuilder::InsertTermToNode(uint64_t address, char *term, unsigned v) {
  DiskItem* DI;
  if (address >= DAM_->getItemCounter()) {
    while(address > DAM_->getItemCounter()) {
      DI = DAM_->addNewItem();
      InitialNode(DI,0);
      delete DI;
    }
    DI = DAM_->addNewItem();
    InitialNode(DI,0);
  } else {
    if (fast_buffer_ != NULL && address < Prefix_Tree_Fast_Buffer_Size) {
      DI = fast_buffer_->GetItem(address);
    } else {
      DI = DAM_->localItem(address);
    }
  }
  PrefixTreeNode* node = (PrefixTreeNode*)DI->content;
  char ch = *term;
  if (ch == '\0') {
    node->value = v;
    delete DI;
    return;
  }
  unsigned i;
  for(i=0; i<node->children_num; i++) {
    if(node->children[i].first == ch) {
      uint64_t new_address = node->children[i].second;
      delete DI;
      InsertTermToNode(new_address, term+1, v);
      return;
    }
  }
  if (node->children_num < Prefix_Tree_Node_Size) {
    node->children[node->children_num].first = ch;
    node->children[node->children_num].second = DAM_->getItemCounter();
    node->children_num++;
    delete DI;
    InsertTermToNode(DAM_->getItemCounter(), term+1, v);
    return;
  }
  if (node->next > 0) {
    uint64_t new_address = node->next;
    delete DI;
    InsertTermToNode(new_address, term, v);
    return;
  } else {
    node->next = DAM_->getItemCounter();
    delete DI;
    InsertTermToNode(DAM_->getItemCounter(), term, v);
    return;
  }
  delete DI;
}

void PrefixTreeBuilder::WriteToFile(char* filePath) {
  DiskAsMemory *new_DAM = new DiskAsMemory(filePath,sizeof(char),4096);
  stack<uint64_t> node_stack;
  stack<pair<DiskMultiItem*, unsigned> > parent_stack;
  node_stack.push(0);

  vector<pair<char, uint64_t> > cur_children;
  unsigned cur_value;
  while(!node_stack.empty()) {
    uint64_t address = node_stack.top();
    node_stack.pop();
    cur_children.clear();
    bool sub_mark = false;
    do {
      DiskItem* DI = DAM_->localItem(address);
      PrefixTreeNode* node = (PrefixTreeNode*)DI->content;
      if (!sub_mark) {
        cur_value = node->value;
        sub_mark = true;
      }
      for (unsigned i=0; i<node->children_num; i++) {
        cur_children.push_back(node->children[i]);
      }
      address = node->next;
      delete DI;
    } while (address > 0);
    unsigned char children_num = cur_children.size();
    // For headnode.
    if (new_DAM->getItemCounter() == 0) {
      DiskItem* DI = new_DAM->addNewItem();
      memcpy(DI->content,&children_num, sizeof(char));
      delete DI;
    }
    unsigned size = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*children_num;
    // Connect it with it parent.
    uint64_t position = new_DAM->getItemCounter();
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
    DiskMultiItem* items = new_DAM->AddMultiNewItem(size);
    items->MemcpyIn((char*)&cur_value,0,sizeof(unsigned));
    if (children_num > 0) {
      parent_stack.push(pair<DiskMultiItem*,unsigned>(items,children_num));
      for(unsigned i=0; i<children_num; i++) {
        unsigned offset = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*i;
        items->MemcpyIn((char*)&cur_children[i].first,offset,sizeof(char));
        node_stack.push(cur_children[i].second);
      }
    } else {
      delete items;
    }
  }
  delete new_DAM;
}
