#include "Parser.hpp"
#include <iostream>
#include <string>

using namespace std;

int main(int argc,char** argv) {
  TRECParser* parser = new TRECParser("testData/test_collection");
  while(parser->NextDocument()) {
    const ParseredDocument& doc = parser->CurrentDocument();
    cout<<doc.doc_name<<":"<<endl;
    vector<string>::const_iterator it;
    for (it = doc.terms.begin(); it != doc.terms.end(); it++) {
      cout<<" "<<*it;
    }
    cout<<endl;
  }
  delete (parser);
  return 0;
}
