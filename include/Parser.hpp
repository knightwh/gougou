#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct ParseredDocument {
  string doc_name;
  vector<string> terms;
};

class TRECParser {
public:
  TRECParser(char* filename);
  TRECParser(string filename);
  const ParseredDocument& CurrentDocument() {return cur_document_;}
  bool NextDocument();
  bool IsOpen() { return file_handle_.is_open();}
  ~TRECParser() { file_handle_.close(); };
private:
  ifstream file_handle_;
  ParseredDocument cur_document_;
};
	
