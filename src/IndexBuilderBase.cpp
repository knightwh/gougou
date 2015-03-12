#include <sys/stat.h>
#include <iostream>

#include "IndexBuilderBase.hpp"
#include "IndexGlobalInfo.hpp"
#include "TermSummaryForInvert.hpp"

using namespace std;

IndexBuilderBase::IndexBuilderBase(char* p) {
  strcpy(path_,p);
  mkdir(p,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  char temp_path[256];
  strcpy(temp_path,path_);
  strcat(temp_path,"/invert_temp");
  invert_index_ = new InvertIndexBuilder(temp_path);
  term_lookup_ = new PrefixTree();
  char prefix_tree_temp_path[256];
  strcpy(prefix_tree_temp_path,path_);
  strcat(prefix_tree_temp_path,"/prefix_tree_temp");
  //term_lookup_ = new PrefixTreeBuilder(prefix_tree_temp_path);
  tf_generator_ = new DocumentTFGenerator();
  char term_name_path[256];
  strcpy(term_name_path,path_);
  strcat(term_name_path,"/term_name_lookup");
  term_name_ = new NameLookup(term_name_path);
  char doc_name_path[256];
  strcpy(doc_name_path,path_);
  strcat(doc_name_path,"/doc_name_lookup");
  doc_name_ = new NameLookup(doc_name_path);
  char doc_index_path[256];
  strcpy(doc_index_path,path_);
  strcat(doc_index_path,"/doc_index");
  doc_index_ = new VectorInfo(doc_index_path);
  char doc_length_path[256];
  strcpy(doc_length_path,path_);
  strcat(doc_length_path,"/doc_length");
  doc_length_ = new SimpleStatBuilder<unsigned>(doc_length_path);
  doc_length_->AddItem(0);
  docID_= 0;
  stemmer_ = new StemTerm();
}

IndexBuilderBase::~IndexBuilderBase() {
  cout << "OK" <<endl;
  delete invert_index_;
  delete term_lookup_;
  delete tf_generator_;
  delete term_name_;
  delete doc_name_;
  delete doc_index_;
  delete stemmer_;
}

void IndexBuilderBase::BuildIndex(char* text_file) {
  SimpleHTMLParser* parser = new SimpleHTMLParser(text_file);
  while (parser->NextDocument()) {
    //IndexCurDocument(tf_generator->GetTF(parser->CurrentDocument()));
    IndexCurDocument(parser->CurrentDocument());
    if (docID_%500 == 0) ShowProcess();
  }
  delete parser;
}

unsigned IndexBuilderBase::LookupTerm(const string& s) {
  unsigned termID;
  termID = term_lookup_->LookupTerm(s);
  if (termID != 0) return termID;
  termID = term_lookup_->InsertTerm(s);
  if (termID > 0) term_name_->AddItem(s,termID);
  return termID;
}

void IndexBuilderBase::IndexCurDocument(const ParseredDocument& doc) {
  docID_++;
  doc_name_->AddItem(doc.doc_name,docID_);
  DocumentTF TF_info = tf_generator_->GetTF(doc);
  vector<pair<string,unsigned> >::iterator it;
  for (it = TF_info.terms.begin(); it != TF_info.terms.end(); it++) {
    unsigned termID = LookupTerm(it->first);
    Posting p;
    p.docID = docID_;
    p.tf = it->second;
    invert_index_->PushPosting(termID,p);
  }
  // index document into document index.
  vector<unsigned> doc_vector;
  vector<string>::const_iterator term_it;
  for (term_it = doc.terms.begin(); term_it != doc.terms.end(); term_it++) {
    string stemmed_term = stemmer_->stem(*term_it);
    unsigned termID = LookupTerm(stemmed_term);
    doc_vector.push_back(termID);
  }
  doc_index_->AddVector(doc_vector,docID_);
  doc_length_->AddItem(doc_vector.size());
}

void IndexBuilderBase::Finalize() {
  // Prefix tree.
  char tree_path[256];
  strcpy(tree_path,path_);
  strcat(tree_path,"/term_lookup");
  term_lookup_->WriteToFile(tree_path);
  // Invert index writing.
  char invert_summary_path[256];
  strcpy(invert_summary_path,path_);
  strcat(invert_summary_path,"/invert_summary");
  char invert_body_path[256];
  strcpy(invert_body_path,path_);
  strcat(invert_body_path,"/invert_body");
  invert_index_->Finalize(invert_summary_path,invert_body_path);
  // Index the global information.
  char global_path[256];
  strcpy(global_path,path_);
  strcat(global_path,"/global");
  DiskAsMemory* DAM = new DiskAsMemory(global_path,sizeof(IndexGlobalInfo),1024);
  DiskItem* item = DAM->addNewItem();
  IndexGlobalInfo* global = (IndexGlobalInfo*)item->content;
  global->docN = doc_name_->GetNum();
  global->termN = term_name_->GetNum();
  global->totalTF = invert_index_->GetTermMeta()->GetTotalTF();
  delete item;
  delete DAM;
}

void IndexBuilderBase::ShowProcess() {
  cout<<"\rProcessed document "<<docID_<<flush;
}
