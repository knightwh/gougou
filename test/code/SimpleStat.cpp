#include <stdlib.h>
#include <ctime>
#include <iostream>

#include "SimpleStatBuilder.hpp"
#include "SimpleStatReader.hpp"

#define TEST_NUM_SIZE (1024*512)

using namespace std;

int main(int argc,char** argv) {
  srand ( unsigned ( std::time(0) ) );
  SimpleStatBuilder<unsigned>* builder = new SimpleStatBuilder<unsigned>("simple_temp");
  for(unsigned i=0;i<TEST_NUM_SIZE;i++) {
    builder->AddItem(i);
  }
  delete builder;

  SimpleStatReader<unsigned>* reader = new SimpleStatReader<unsigned>("simple_temp");
  unsigned num = reader->GetNum();
  cout<<"num="<<num<<" vs "<<TEST_NUM_SIZE<<endl;
  for(unsigned i=0;i<100;i++) {
    unsigned n = rand()%num;
    cout<<reader->GetItem(n)<<" vs "<<n<<endl;
  }
  delete reader;
    
  
  return 0;
}
  
