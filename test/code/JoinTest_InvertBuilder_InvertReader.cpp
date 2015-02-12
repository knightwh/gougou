#include "InvertIndexBuilder.hpp"
#include "InvertIndexReader.hpp"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <vector>

#define TEST_SIZE 100000
#define TERM_SIZE 100

#include "Posting.hpp"

using namespace std;

int main(int argc, char** argv) {
  srand ( unsigned ( std::time(0) ) );
  vector<vector<Posting> > res;
  InvertIndexBuilder* IIB = new InvertIndexBuilder("invert_temp");
  cout << "OK" <<endl;
  vector<unsigned> cur_docID;
  for(unsigned i=0; i<TEST_SIZE; i++) {
    unsigned termID = rand() % (res.size() + 1);
    if (termID >= res.size()) {
      res.resize(termID+1);
      cur_docID.resize(termID+1);
      cur_docID[termID] = 0;
    }
    if (termID == 0) continue;
    Posting p;
    unsigned delta = rand() % 100000+1;
    p.docID = cur_docID[termID] + delta;
    cur_docID[termID] = p.docID;
    p.tf = rand() % 100+1;
    res[termID].push_back(p);
    IIB->PushPosting(termID,p);
  }
  IIB->Finalize("invert_summary","invert_body");
  delete IIB;

  // Begin test the index reader.
  InvertIndexReader* IIR = new InvertIndexReader("invert_summary","invert_body");
  for (unsigned i=1; i<res.size(); i++) {
    PostingListReader* PLR = IIR->GetPosting(i);
    if (res[i].size() != PLR->GetSummary().df) cout<< res[i].size() << " VS " << PLR->GetSummary().df << endl;
    unsigned num = 0;
    while (PLR -> NextDoc()) {
      if (res[i][num].docID != PLR->CurDocID()) cout << "docID: " << res[i][num].docID << " vs " << PLR->CurDocID() << endl;
      if (res[i][num].tf != PLR->CurTF()) cout << "tf: " << res[i][num].tf << " vs " << PLR->CurTF() << endl;
      num++;
    }
    delete PLR;
  }
  delete IIR;
  return 0;
}
