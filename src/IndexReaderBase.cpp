#include "IndexReaderBase.hpp"

#include <iostream>
#include "DAMreader.hpp"
#include "IndexGlobalInfo.hpp"

using namespace std;

IndexReaderBase::IndexReaderBase(char* path) {
  // Get the global information.
  char global_path[256];
  strcpy(global_path,path);
  strcat(global_path,"/global");
  DiskAsMemoryReader* DAM = new DiskAsMemoryReader(global_path);
  DiskItemReader* item = DAM->localItem(0);
  IndexGlobalInfo* global = (IndexGlobalInfo*)item->content;
  doc_num_ = global->docN;
  total_term_num_ = global->totalTF;
  term_num_ = global->termN;
  doc_length_avg_ = (double)total_term_num_ / doc_num_;
  delete item;
  delete DAM;
  // For InvertIndex Reader.
  char invert_summary_path[256];
  char invert_body_path[256];
  strcpy(invert_summary_path,path);
  strcat(invert_summary_path,"/invert_summary");
  strcpy(invert_body_path,path);
  strcat(invert_body_path,"/invert_body");
  invert_index_ = new InvertIndexReader(invert_summary_path,invert_body_path);
  // For document and term name lookup.
  char term_name_path[256];
  strcpy(term_name_path,path);
  strcat(term_name_path,"/term_name_lookup");
  term_name_lookup_ = new NameLookupReader(term_name_path);
  char doc_name_path[256];
  strcpy(doc_name_path,path);
  strcat(doc_name_path,"/doc_name_lookup");
  doc_name_lookup_ = new NameLookupReader(doc_name_path);
  // Document index.
  char doc_index_path[256];
  strcpy(doc_index_path,path);
  strcat(doc_index_path,"/doc_index");
  doc_index_ = new VectorInfoReader(doc_index_path);
  // Document length fast lookup.
  char doc_length_path[256];
  strcpy(doc_length_path,path);
  strcat(doc_length_path,"/doc_length");
  doc_length_ = new SimpleStatReader<unsigned>(doc_length_path);
  // Stemmer.
  stemmer_ = new StemTerm();
  // PrefixTree.
  char term_lookup_path[256];
  strcpy(term_lookup_path,path);
  strcat(term_lookup_path,"/term_lookup");
  term_lookup_ = new PrefixTreeReader(term_lookup_path);
}

IndexReaderBase::~IndexReaderBase() {
  delete invert_index_;
  delete term_name_lookup_;
  delete doc_name_lookup_;
  delete doc_index_;
  delete stemmer_;
  delete term_lookup_;
}

PostingListReader* IndexReaderBase::GetPosting(unsigned n) {
  return invert_index_->GetPosting(n);
}

PostingListReader* IndexReaderBase::GetPosting(const string& s) {
  unsigned termID = term_lookup_->LookupTerm(stemmer_->stem(s));
  if (termID == 0) return NULL;
  return invert_index_->GetPosting(termID);
}

PostingListReader* IndexReaderBase::GetPosting(char* term) {
  unsigned termID = term_lookup_->LookupTerm(stemmer_->stem(term));
  if (termID == 0) return NULL;
  return invert_index_->GetPosting(termID);
}

string IndexReaderBase::TermName(unsigned ID) {
  return term_name_lookup_->Lookup(ID);
}

string IndexReaderBase::DocName(unsigned ID) {
  return doc_name_lookup_->Lookup(ID);
}

vector<unsigned> IndexReaderBase::GetDocVector(unsigned ID) {
  return doc_index_->GetVector(ID);
}
