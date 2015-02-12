#include "SimpleStatReader.hpp"

#include <stdint.h>
#include "DAMreader.hpp"

using namespace std;

template <class T>
SimpleStatReader<T>::SimpleStatReader(char* path) {
  DiskAsMemoryReader* DAM = new DiskAsMemoryReader(path);
  num_ = DAM->getItemCounter();
  data_ = DAM->CopyItems(0,num_);
  delete DAM;
  items_ = (T*)data_;
}

template <class T>
SimpleStatReader<T>::~SimpleStatReader() {
  delete[] data_;
}

template <class T>
T SimpleStatReader<T>::GetItem(unsigned item) {
  if (item >= num_) return items_[0];
  return items_[item];
}
