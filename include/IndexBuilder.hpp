#ifndef GOUGOU_INDEX_BUILDER_
#define GOUGOU_INDEX_BUILDER_

#include <iostream>
#include <string>

#include "IndexBuilderBase.hpp"

using namespace std;

class IndexBuilder {
public:
  void ProcessFile(char* filename);
  void ProcessFile(string filename) { ProcessFile(filename.c_str()); }
  void BuildIndex();
  void Show(ostream& is);
  string RemoveSpace(const string& str);
private:
  string index_path_;
  vector<string> data_path_;
};

#endif
