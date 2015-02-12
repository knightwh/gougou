#ifndef GOUGOU_DISK_DATA_Reader_
#define GOUGOU_DISK_DATA_Reader_

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>

using namespace std;

class DiskDataReader {
 public:
  DiskDataReader(unsigned length) {
    length_=length;
    pos_=0;
  }
  inline void GetMapBuffer(char* buffer) { buffer_ = buffer;}
  inline void AddData(char* buffer,unsigned length) {
    if (pos_ == 0) buffer_ = new char[length_];
    strcpy(buffer_+pos_, buffer, length);
    pos_ += length;
  }
  inline char* GetBuffer() {return buffer_;}
  ~DiskDataReader() {
    if (pos_ == 0) munmap(buffer,length);
    else delete[] buffer_;
  }
 private:
  char* buffer_;
  unsigned length_;
  unsigned pos_;
};

#endif
