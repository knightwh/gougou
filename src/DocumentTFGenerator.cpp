#include <iostream>
#include "DocumentTFGenerator.hpp"

using namespace std;

DocumentTF DocumentTFGenerator::GetTF(const ParseredDocument& doc) {
  map<string,unsigned> term_tab;
  map<string,unsigned>::iterator term_it;
  vector<string>::const_iterator doc_it;
  for(doc_it = doc.terms.begin(); doc_it != doc.terms.end(); doc_it++) {
    string stemmed_term = stemmer->stem(*doc_it);
    term_it = term_tab.find(stemmed_term);
    if (term_it != term_tab.end()) {
      term_it->second++;
    } else {
      term_tab.insert(pair<string,unsigned>(stemmed_term,1));
    }
  }
  DocumentTF tf;
  tf.doc_name = doc.doc_name;
  for (term_it = term_tab.begin(); term_it != term_tab.end(); term_it++) {
    tf.terms.push_back(*term_it);
  }
  return tf;
}
