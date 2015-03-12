#ifndef GOUGOU_INDEX_BUILDER_BASE_
#define GOUGOU_INDEX_BUILDER_BASE_

#include "DAM.hpp"
#include "DocumentTFGenerator.hpp"
#include "InvertIndexBuilder.hpp"
#include "NameLookup.hpp"
#include "Parser.hpp"
#include "porter.hpp"
#include "PrefixTree.hpp"
//#include "PrefixTreeBuilder.hpp"
#include "SimpleStatBuilder.hpp"
#include "TermMeta.hpp"
#include "VectorInfo.hpp"

using namespace std;

class IndexBuilderBase {
 public:
  IndexBuilderBase(char* path);
  void BuildIndex(char* text_file);
  void ShowProcess();
  void Finalize();
  ~IndexBuilderBase();
 protected:
  void IndexCurDocument(const ParseredDocument&);
  unsigned LookupTerm(const string& s);
 private:
  InvertIndexBuilder* invert_index_;
  //PrefixTreeBuilder* term_lookup_;
  PrefixTree* term_lookup_;
  DocumentTFGenerator* tf_generator_;
  NameLookup* term_name_;
  NameLookup* doc_name_;
  VectorInfo* doc_index_;
  SimpleStatBuilder<unsigned>* doc_length_;
  char path_[256];
  unsigned docID_;
  StemTerm* stemmer_;
};

#endif
