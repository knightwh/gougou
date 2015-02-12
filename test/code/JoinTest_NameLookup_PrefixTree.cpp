#include <iostream>
#include <map>

#include "NameLookup.hpp"
#include "NameLookupReader.hpp"
#include "PrefixTree.hpp"
#include "PrefixTreeReader.hpp"
#include "Parser.hpp"
#include "DocumentTFGenerator.hpp"

using namespace std;

int main(int argc,char** argv) {
  map<string,unsigned> res;
  TRECParser* parser = new TRECParser("/infolab/infolab0/haowu/trec-collections/data/doe.trec");
  //TRECParser* parser = new TRECParser("data/test_collection");
  DocumentTFGenerator *TF_generator = new DocumentTFGenerator();
  NameLookup *NL = new NameLookup("temp-NameLookup");
  PrefixTree *PT = new PrefixTree();

  cout<<"OK"<<endl;
  unsigned num = 0;
  map<string,unsigned>::iterator map_it;
  while(parser->NextDocument()) {
    DocumentTF TF_info = TF_generator->GetTF(parser->CurrentDocument());
    //cout<<TF_info.doc_name<<endl;
    vector<pair<string,unsigned> >::iterator it;
    for (it = TF_info.terms.begin(); it != TF_info.terms.end(); it++) {
      //if(it->first.length()==0) continue;
      unsigned termID = PT->InsertTerm(it->first);
      if(termID > num) {
        NL->AddItem(it->first, termID);
        res.insert(pair<string,unsigned>(it->first,termID));
        num = termID;
      }
    }
  }
  delete NL;
  PT->WriteToFile("temp-prefixTree");
  delete PT;
  
  // Begin test the result;
  cout<<"Begin Test Reading"<<endl;
  NameLookupReader* NLR = new NameLookupReader("temp-NameLookup");
  PrefixTreeReader* PTR = new PrefixTreeReader("temp-prefixTree");
  for (map_it=res.begin(); map_it!=res.end(); map_it++) {
    unsigned ID = PTR->LookupTerm(map_it->first);
    string name = NLR->Lookup(ID);
    if(map_it->first != name) cout<<map_it->second<<"\t"<<map_it->first<<" vs "<<name<<endl;
  }
  delete parser;
  delete TF_generator;
  delete NLR;
  delete PTR;
  return 0;
}
