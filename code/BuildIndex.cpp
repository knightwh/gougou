#include <iostream>

#include "IndexBuilderBase.hpp"

using namespace std;

int main(int argc, char** argv) {
  if(argc != 3) {
    cerr<<"Usage: "<<argv[0]<<" dataPath indexPath\n";
    return 0;
  }
  IndexBuilderBase* IBB=new IndexBuilderBase(argv[2]);
  IBB->BuildIndex(argv[1]);
  cout<<"...Finalize the job!"<<endl;
  IBB->Finalize();
  cout<<"Done!"<<endl;
  delete IBB;
  
  return 0;
}
