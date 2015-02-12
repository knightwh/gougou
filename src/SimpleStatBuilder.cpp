#include "SimpleStatBuilder.hpp"

#include "DAM.hpp"

using namespace std;

template <typename T>
SimpleStatBuilder<T>::SimpleStatBuilder(char* path) {
  DAM_ = new DiskAsMemory(path,sizeof(T),4096*16);
}

template <typename T>
SimpleStatBuilder<T>::~SimpleStatBuilder() {
  delete DAM_;
}

template <typename T>
void SimpleStatBuilder<T>::AddItem(T const& con) {
  DiskItem* DI = DAM_->addNewItem();
  memcpy(DI->content,&con,sizeof(T));
  delete DI;
}

template <typename T>
T SimpleStatBuilder<T>::GetItem(uint64_t item) {
  DiskItem* DI = DAM_->localItem(item);
  T res;
  memcpy(&res,DI->content,sizeof(T));
  delete DI;
  res;
}
