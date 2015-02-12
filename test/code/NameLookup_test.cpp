#include <iostream>
#include <map>

#include "NameLookup.hpp"
#include "NameLookupReader.hpp"
#include "Parser.hpp"
#include "DocumentTFGenerator.hpp"

using namespace std;

int main(int argc,char** argv) {
  map<string,unsigned> res;
  //TRECParser* parser = new TRECParser("/infolab/infolab0/haowu/trec-collections/data/doe.trec");
  TRECParser* parser = new TRECParser("data/test_collection");
  DocumentTFGenerator *TF_generator = new DocumentTFGenerator();
  NameLookup *NL = new NameLookup("temp-NameLookup");

  cout<<"OK"<<endl;
  map<string,unsigned>::iterator map_it;
  unsigned num = 0;
  while(parser->NextDocument()) {
    DocumentTF TF_info = TF_generator->GetTF(parser->CurrentDocument());
    //cout<<TF_info.doc_name<<endl;
    vector<pair<string,unsigned> >::iterator it;
    for (it = TF_info.terms.begin(); it != TF_info.terms.end(); it++) {
      if(it->first.length()==0) continue;
      map_it = res.find(it->first);
      if(map_it == res.end()) {
        NL->AddItem(it->first, ++num);
        res.insert(pair<string,unsigned>(it->first,num));
      }
    }
  }
  delete NL;
  
  // Begin test the result;
  cout<<"Begin Test Reading"<<endl;
  NameLookupReader* NLR = new NameLookupReader("temp-NameLookup");
  for (map_it=res.begin(); map_it!=res.end(); map_it++) {
    string name = NLR->Lookup(map_it->second);
    if(map_it->first != name) cout<<map_it->second<<"\t"<<map_it->first<<" vs "<<name<<endl;
  }
  delete parser;
  delete TF_generator;
  delete NLR;
  return 0;
}
