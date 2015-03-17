#include "InvertIndexBuilder.hpp"

#include "DAM.hpp"
#include "TermMeta.hpp"
#include "TermSummaryForInvert.hpp"

using namespace std;

InvertIndexBuilder::InvertIndexBuilder(char* temp_path) {
  term_meta_ = new TermMetaManager(temp_path);
}

InvertIndexBuilder::~InvertIndexBuilder() {
  delete term_meta_;
}

void InvertIndexBuilder::Finalize(char* term_path, char* invert_path) {
  term_meta_->Finalize();
  DiskAsMemory* invert_DAM = new DiskAsMemory(invert_path,sizeof(unsigned),4096);
  DiskAsMemory* term_DAM = new DiskAsMemory(term_path,sizeof(TermSummaryForInvert),1024);
  DiskItem* DI = term_DAM->addNewItem();
  TermSummaryForInvert* total_summary = (TermSummaryForInvert*)DI->content;
  total_summary->tf = term_meta_->GetTotalTF();
  total_summary->df = term_meta_->GetTermSize();
  total_summary->block_num = 0;
  total_summary->address = 0;
  total_summary->data_length = 0;
  delete DI;
  for (unsigned i=1; i<term_meta_->GetTermSize(); i++) {
    // Fill up the term summary part.
    DI = term_DAM->addNewItem();
    TermSummaryForInvert* summary = (TermSummaryForInvert*)DI->content;
    DiskItem* meta_item = term_meta_->GetTermMeta(i);
    TermMeta* meta = (TermMeta*)meta_item->content;
    //TermMeta* term_data = term_meta_->GetTermMeta(i);
    summary->df = meta->df;
    summary->tf = meta->tf;
    summary->block_num = term_meta_->GetBlockNum(i);
    summary->address = invert_DAM->getItemCounter();
    delete meta_item;
    // Fill up the block information part of invert index.
    TermMetaIterator* it = term_meta_->GetBlockIterator(i);
    //const vector<TermMetaBlock>& blocks = term_data->GetBlock();
    DiskMultiItem* items = invert_DAM->AddMultiNewItem(summary->block_num*sizeof(BlockSummaryForInvert)/sizeof(unsigned));
    //for (unsigned l=0; l<blocks.size(); l++) {
    unsigned offset = 0;
    while(it->Next()) {
      TermMetaBlock block = it->Current();
      BlockSummaryForInvert block_summary;
      block_summary.a1 = block.a1;
      block_summary.b1 = block.b1;
      block_summary.length1 = block.length1;
      block_summary.a2 = block.a2;
      block_summary.b2 = block.b2;
      block_summary.length2 = block.length2;
      block_summary.docID = block.docID;
      items->MemcpyIn((char*)&block_summary,offset,sizeof(BlockSummaryForInvert)/sizeof(unsigned));
      offset += sizeof(BlockSummaryForInvert)/sizeof(unsigned);
    }
    delete items;
    delete it;
    // Fill up the main body of invert index.
    it = term_meta_->GetBlockIterator(i);
    //for (unsigned l=0; l<blocks.size(); l++) {
    while(it->Next()) {
      TermMetaBlock block = it->Current();
      items = invert_DAM->AddMultiNewItem(block.length1);
      DiskMultiItem* source_items = term_meta_->GetDAM()->LocalMultiItem(block.address1,block.length1);
      unsigned temp1[block.length1];
      source_items->MemcpyOut((char*)temp1,0,block.length1);
      items->MemcpyIn((char*)temp1,0,block.length1);
      delete items;
      delete source_items;
      items = invert_DAM->AddMultiNewItem(block.length2);
      source_items = term_meta_->GetDAM()->LocalMultiItem(block.address1+block.length1,block.length2);
      unsigned temp2[block.length2];
      source_items->MemcpyOut((char*)temp2,0,block.length2);

      items->MemcpyIn((char*)temp2,0,block.length2);
      delete items;
      delete source_items;
    }
    summary->data_length = invert_DAM->getItemCounter() - summary->address;
    delete DI;
    delete it;
  }
  delete invert_DAM;
  delete term_DAM;
}
