#ifndef GOUGOU_DOCUMENT_TF_GENERATOR
#define GOUGOU_DOCUMENT_TF_GNEERATOR

#include <map>
#include <string>

#include "porter.hpp"
#include "Parser.hpp"

using namespace std;

struct DocumentTF {
  string doc_name;
  vector<pair<string,unsigned> > terms;
};

class DocumentTFGenerator {
 public:
  DocumentTFGenerator() {
    stemmer = new StemTerm();
  }
  DocumentTF GetTF(const ParseredDocument&);
  ~DocumentTFGenerator() { delete stemmer;}
 private:
  StemTerm* stemmer;
};

#endif
