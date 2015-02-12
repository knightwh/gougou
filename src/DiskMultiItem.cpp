#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "DiskMultiItem.hpp"

using namespace std;

DiskMultiItem::DiskMultiItem(uint64_t begin_item,unsigned item_size) {
  begin_item_ = begin_item;
  item_size_ = item_size;
  item_num_ = 0;
}

DiskItem* DiskMultiItem::GetItemRel(unsigned item) const {
  if (item>=item_num_) return NULL;
  uint64_t num = begin_item_+item;
  for(unsigned i=0; i<unit_.size(); i++) {
    if(item < unit_[i].item_num) {
      return unit_[i].DB->localItem(num);
    }
    item -= unit_[i].item_num;
  }
  return NULL;
}

DiskItem* DiskMultiItem::GetItem(uint64_t item) const {
  if (item >= begin_item_ + item_num_) return NULL;
  return GetItemRel(item - begin_item_);
}

bool DiskMultiItem::MemcpyIn(char* source,unsigned offset,unsigned copy_item_num) {
  if (offset+copy_item_num > item_num_) return false;
  for (unsigned i=0; i<unit_.size(); i++) {
    if (offset < unit_[i].item_num) {
      if (copy_item_num == 0) break;
      int cnum = copy_item_num;
      if (cnum > unit_[i].item_num-offset) cnum = unit_[i].item_num-offset;
      memcpy(unit_[i].buffer+offset*item_size_, source, cnum*item_size_);
      source += cnum*item_size_;
      copy_item_num -= cnum;
      offset = 0;
    } else {
      offset -= unit_[i].item_num;
    }
  }
  return (copy_item_num == 0);
}

bool DiskMultiItem::MemcpyOut(char* destiny,unsigned offset,unsigned copy_item_num)  const {
  if (offset+copy_item_num > item_num_) return false;
  for (unsigned i=0; i<unit_.size(); i++) {
    if (offset < unit_[i].item_num) {
      if (copy_item_num == 0) break;
      int cnum = copy_item_num;
      if (cnum > unit_[i].item_num-offset) cnum = unit_[i].item_num-offset;
      memcpy(destiny, unit_[i].buffer+offset*item_size_, cnum*item_size_);
      destiny += cnum*item_size_;
      copy_item_num -= cnum;
      offset = 0;
    } else {
      offset -= unit_[i].item_num;
    }
  }
  return (copy_item_num == 0);
}

bool DiskMultiItem::PutItems(DiskBuffer* DB,char* buffer,unsigned item_num) {
  DiskMultiItemUnit u;
  if (buffer == NULL) return false;
  u.buffer = buffer;
  u.item_num = item_num;
  u.DB = DB;
  unit_.push_back(u);
  item_num_ += item_num;
  return true;
}

DiskMultiItem::~DiskMultiItem() {
  for(unsigned i=0; i<unit_.size(); i++) {
    unit_[i].DB->freeUsage();
  }
}
