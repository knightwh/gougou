#include "IndexReaderBase.hpp"

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  IndexReaderBase* IR = new IndexReaderBase("index_temp");
  cout<<"DocCount:"<<IR->DocCount()<<endl;
  cout<<"TotalTerm:"<<IR->TotalTermCount()<<endl;
  cout<<"TermCount:"<<IR->TermCount()<<endl;
  cout<<"Average Doc Length:"<<IR->DocLengthAvg()<<endl;
  // Test Document Vector.
  vector<unsigned> doc_vector = IR->GetDocVector(1000);
  for (unsigned i=0; i< IR->DocLength(1000); i++) {
    cout<<" "<<IR->TermName(doc_vector[i]);
  }
  cout<<endl;
  // Test invert index.
  PostingListReader* PLR = IR->GetPosting("profit");
  while(PLR->NextDoc()) {
    cout<<IR->DocName(PLR->CurDocID())<<"\t"<<PLR->CurTF()<<endl;
  }
  delete PLR;
  delete IR;
  return 0;
}
