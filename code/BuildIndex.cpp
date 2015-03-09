#include "IndexBuilder.hpp"

using namespace std;

int main(int argc,char** argv) {
  if (argc < 2) {
    cerr<<"Usage: "<<argv[0]<<" paraFile1 paraFile2 ..."<<endl;
    return 0;
  }
  IndexBuilder* IB = new IndexBuilder();
  for(unsigned i=1; i < argc; i++) {
    IB->ProcessFile(argv[i]);
  }
  IB->BuildIndex();
  delete IB;
  return 0;
  
}
