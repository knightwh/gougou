#include <iostream>

#include "IndexBuilderBase.hpp"

using namespace std;

int main(int argc, char** argv) {
  IndexBuilderBase* IBB=new IndexBuilderBase("index_temp");
  //IBB->BuildIndex("data/test_collection");
  IBB->BuildIndex("/infolab/infolab0/haowu/trec-collections/data/disk45.trec");
  cout<<"Hello World!"<<endl;
  IBB->Finalize();
  delete IBB;
  
  return 0;
}
