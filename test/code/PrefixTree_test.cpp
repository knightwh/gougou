#include <iostream>
#include <map>

//#include "PrefixTree.hpp"
#include "PrefixTreeMix.hpp"
#include "PrefixTreeReader.hpp"
#include "Parser.hpp"
#include "DocumentTFGenerator.hpp"

using namespace std;

int main(int argc,char** argv) {
  map<string,unsigned> res;
  TRECParser* parser = new TRECParser("/infolab/infolab0/haowu/trec-collections/data/doe.trec");
  //TRECParser* parser = new TRECParser("data/test_collection");
  DocumentTFGenerator *TF_generator = new DocumentTFGenerator();
  //PrefixTree *PT = new PrefixTree();
  PrefixTreeMix *PT = new PrefixTreeMix("../../gougou_test/tree_test");
  while(parser->NextDocument()) {
    DocumentTF TF_info = TF_generator->GetTF(parser->CurrentDocument());
    //cout<<TF_info.doc_name<<endl;
    vector<pair<string,unsigned> >::iterator it;
    for (it = TF_info.terms.begin(); it != TF_info.terms.end(); it++) {
      unsigned termID = 0;
      termID = PT->LookupTerm(it->first.c_str());
      if(termID == 0) {
        termID = PT->InsertTerm(it->first);
        //termID = PT->InsertTerm("ab");
        if (termID % 500 == 0) cout<<termID<<endl;
      }
      //cout<<it->first<<"\t"<<it->second<<"\t"<<termID<<endl;
      res.insert(pair<string,unsigned>(it->first,termID));
    }
  }
  // Begin test the result;
  map<string,unsigned>::iterator it;
  for (it=res.begin(); it!=res.end(); it++) {
    unsigned termID = PT->LookupTerm(it->first);
    if(it->second != termID) cout<<it->first<<"\t"<<it->second<<" vs "<<termID<<endl;
  }
  PT->WriteToFile("temp-prefixTree");
  delete parser;
  delete TF_generator;
  delete PT;
  // Begin test the PrefixTreeReader.
  cout<<"Begin test the PrefixTreeReader\n\n\n\n"<<endl;
  PrefixTreeReader* PTR = new PrefixTreeReader("temp-prefixTree");
  for (it=res.begin(); it!=res.end(); it++) {
    unsigned termID = PTR->LookupTerm(it->first);
    if(it->second != termID) cout<<it->first<<"\t"<<it->second<<" vs "<<termID<<endl;
  }
  delete PTR;
  return 0;
}
