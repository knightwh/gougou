#include "PrefixTreeMix.hpp"

#include <stack>

using namespace std;

PrefixTreeMix::PrefixTreeMix(char* path) {
  DAM_ = new DiskAsMemory(path,sizeof(PrefixTreeNodeItem), 1024);
  DiskItem* item = DAM_->addNewItem();
  InitialNode(item,0);
  delete item;
  vsize_ = 0;
  head_ = new PrefixTreeNodeMem;
  head_->value = 0;
}

PrefixTreeMix::~PrefixTreeMix() {
  DAM_->truncateItem(0);
  delete DAM_;
  delete head_;
}

void PrefixTreeMix::InitialNode(DiskItem* item, unsigned v) const {
  PrefixTreeNodeItem* content = (PrefixTreeNodeItem*)item->content;
  content->value = v;
  content->children_num = 0;
  content->next = 0;
}

unsigned PrefixTreeMix::LookupTerm(char* term) {
  if(*term == '\0') return 0;
  return LookupTermFromNode(head_,term,0);
}

unsigned PrefixTreeMix::LookupTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return LookupTerm(term);
}

void PrefixTreeMix::InsertTerm(char* term, unsigned v) {
  if(*term == '\0') return;
  InsertTermToNode(head_, term, v, 0);
  if(vsize_ < v) vsize_ = v;
}

void PrefixTreeMix::InsertTerm(string s,unsigned v) {
  char term[s.length()];
  strcpy(term,s.c_str());
  InsertTerm(term,v);
}

unsigned PrefixTreeMix::InsertTerm(char* term) {
  if(*term == '\0') return 0;
  unsigned termID = LookupTerm(term);
  if (termID == 0) {
    InsertTerm(term, ++vsize_);
    return vsize_;
  } else {
    return termID;
  }
}

unsigned PrefixTreeMix::InsertTerm(string s) {
  char term[s.length()];
  strcpy(term,s.c_str());
  return InsertTerm(term);
}

unsigned PrefixTreeMix::LookupTermFromNode(PrefixTreeNodeMem* node, char *term, unsigned level) {
  if (*term == '\0') return node->value;
  vector<pair<char, union PrefixAddress> >::iterator it;
  for(it=node->children.begin(); it!=node->children.end(); it++) {
    if (*term == it->first) {
      if (level < PREFIX_TREE_MEMORY_LEVEL)
        return LookupTermFromNode(it->second.pointer, term+1, level+1);
      else
        return LookupTermFromNode(it->second.disk, term+1);
    }
  }
  return 0;
}

void PrefixTreeMix::InsertTermToNode(PrefixTreeNodeMem* node, char *term, unsigned v,unsigned level) {
  if (*term == '\0') {node->value = v; return; }
  vector<pair<char, union PrefixAddress> >::iterator it;
  for(it=node->children.begin(); it!=node->children.end(); it++) {
    if (*term == it->first) {
      if (level < PREFIX_TREE_MEMORY_LEVEL)
        InsertTermToNode(it->second.pointer, term+1, v, level+1);
      else
        InsertTermToNode(it->second.disk, term+1, v);
      return;
    }
  }
  if (level < PREFIX_TREE_MEMORY_LEVEL) {
    PrefixAddress child;
    child.pointer = new PrefixTreeNodeMem;
    child.pointer->value = 0;
    node->children.push_back(pair<char, PrefixAddress>(*term, child));
    InsertTermToNode(child.pointer, term+1, v, level+1);
  } else {
    PrefixAddress child;
    child.disk = DAM_->getItemCounter();
    node->children.push_back(pair<char, PrefixAddress>(*term, child));
    InsertTermToNode(DAM_->getItemCounter(), term+1, v);
  }
}

unsigned PrefixTreeMix::LookupTermFromNode(uint64_t address, char* term) {
  if (address >= DAM_->getItemCounter()) return 0;
  DiskItem* DI;
  DI = DAM_->localItem(address);
  PrefixTreeNodeItem* node = (PrefixTreeNodeItem*)DI->content;
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

void PrefixTreeMix::InsertTermToNode(uint64_t address, char *term, unsigned v) {
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
    DI = DAM_->localItem(address);
  }
  PrefixTreeNodeItem* node = (PrefixTreeNodeItem*)DI->content;
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
  if (node->children_num < PREFIX_TREE_NODE_SIZE) {
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
}

void PrefixTreeMix::WriteToFile(char* filePath) {
  DiskAsMemory *new_DAM = new DiskAsMemory(filePath, sizeof(char), 4096);
  stack<pair<union PrefixAddress, unsigned> > node_stack;
  stack<pair<DiskMultiItem*, unsigned> > parent_stack;
  PrefixAddress temp_address;
  temp_address.pointer = head_;
  node_stack.push(pair<PrefixAddress, unsigned>(temp_address,0));
  DiskItem* DI = new_DAM->addNewItem();
  unsigned char head_children_num = head_->children.size();
  memcpy(DI->content,&head_children_num,sizeof(char));
  delete DI;

  vector<pair<char, union PrefixAddress> > cur_children;
  unsigned cur_value;
  while(!node_stack.empty()) {
    unsigned level = node_stack.top().second;
    if(level <= PREFIX_TREE_MEMORY_LEVEL) {
      PrefixTreeNodeMem* node = node_stack.top().first.pointer;
      cur_value = node->value;
      cur_children = node->children;
    } else {
      uint64_t address = node_stack.top().first.disk;
      cur_children.clear();
      bool sub_mark = false;
      do {
        DI = DAM_->localItem(address);
        PrefixTreeNodeItem* node = (PrefixTreeNodeItem*)DI->content;
        if (!sub_mark) {
          cur_value = node->value;
          sub_mark = true;
        }
        for (unsigned i=0; i<node->children_num; i++) {
          pair<char, PrefixAddress> child;
          child.first = node->children[i].first;
          child.second.disk = node->children[i].second;
          cur_children.push_back(child);
        }
        address = node->next;
        delete DI;
      } while (address > 0);
    }
    node_stack.pop();
    unsigned char children_num = cur_children.size();
    unsigned size = sizeof(unsigned)+(sizeof(uint64_t)+sizeof(char)*2)*children_num;
    // Connect it with its parent.
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
        node_stack.push(pair<union PrefixAddress, unsigned>(cur_children[i].second,level+1));
      }
    } else {
      delete items;
    }
  }
  delete new_DAM;
}

