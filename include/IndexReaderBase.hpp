#ifndef GOUGOU_INDEX_READER_BASE_
#define GOUGOU_INDEX_READER_BASE_

#include <stdint.h>

#include "DAMreader.hpp"
#include "InvertIndexReader.hpp"
#include "NameLookupReader.hpp"
#include "porter.hpp"
#include "PrefixTreeReader.hpp"
#include "SimpleStatReader.hpp"
#include "VectorInfoReader.hpp"

using namespace std;

class IndexReaderBase {
 public:
  IndexReaderBase(char* path);
  ~IndexReaderBase();
  // For the global information.
  inline unsigned DocCount() const { return doc_num_; }
  inline uint64_t TotalTermCount() const { return total_term_num_; }
  inline unsigned TermCount() const {return term_num_; }
  inline double DocLengthAvg() const { return doc_length_avg_; }
  // For InvertIndexReader.
  PostingListReader* GetPosting(unsigned n);
  PostingListReader* GetPosting(const string& s);
  PostingListReader* GetPosting(char* term);
  // For Document and term name lookup.
  string TermName(unsigned ID);
  string DocName(unsigned ID);
  // For Document Index.
  //inline unsigned DocLength(unsigned ID) {return doc_index_->GetLength(ID);}
  inline unsigned DocLength(unsigned ID) {return doc_length_->GetItem(ID);}
  vector<unsigned> GetDocVector(unsigned ID);
  // For PrefixTree.
  inline unsigned TermLookup(const string& s) { return term_lookup_->LookupTerm(stemmer_->stem(s)); }
  inline unsigned TermLookup(char* term) { return term_lookup_->LookupTerm(stemmer_->stem(term)); }
 private:
  // Global information.
  unsigned doc_num_;
  uint64_t total_term_num_;
  unsigned term_num_;
  double doc_length_avg_;
  // Invert index.
  InvertIndexReader* invert_index_;
  // Document and term name lookup.
  NameLookupReader* term_name_lookup_;
  NameLookupReader* doc_name_lookup_;
  // Document index.
  VectorInfoReader* doc_index_;
  // Document Length fast lookup.
  SimpleStatReader<unsigned>* doc_length_;
  // Stemmer.
  StemTerm* stemmer_;
  // PrefixTree.
  PrefixTreeReader* term_lookup_;
};

#endif
