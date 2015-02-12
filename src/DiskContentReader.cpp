#include "DiskContentReader.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>

using namespace std;

DiskContentReader::DiskContentReader(char* buffer,int offset,unsigned item_size, unsigned item_num) {
  buffer_=buffer;
  item_size_=item_size;
  item_num_=item_num;
  offset_=offset;
  if (offset>=0) con_=buffer+offset;
  else con_=buffer;
}

DiskContentReader::~DiskContentReader() {
  if (offset_>=0) {
    munmap(buffer_,item_size_*item_num_+offset_);
  } else {
    delete[] buffer_;
  }
}
