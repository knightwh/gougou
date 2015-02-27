#ifndef GOUGOU_PARSER_
#define GOUGOU_PARSER_

#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct ParseredDocument {
  string doc_name;
  vector<string> terms;
};

class Parser {
public:
  Parser(char* filename);
  Parser(string filename) { Parser(filename.c_str()); }
  const ParseredDocument& CurrentDocument() {return cur_document_;}
  virtual bool NextDocument() {return true;};
  bool IsOpen() { return file_handle_.is_open();}
  ~Parser() { file_handle_.close(); }
protected:
  ifstream file_handle_;
  ParseredDocument cur_document_;
};

class TRECParser : public Parser {
public:
  virtual bool NextDocument();
  TRECParser(char* filename) : Parser(filename) {};
  TRECParser(string filename) : Parser(filename) {};
};

class SimpleHTMLParser : public Parser {
public:
  virtual bool NextDocument();
  SimpleHTMLParser(char* filename) : Parser(filename) {};
  SimpleHTMLParser(string filename) : Parser(filename) {};
};

#endif
