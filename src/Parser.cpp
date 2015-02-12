#include "Parser.hpp"

#include "string.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

using namespace std;

TRECParser::TRECParser(string filename) {
  TRECParser(filename.c_str());
}

TRECParser::TRECParser(char* filename) {
  file_handle_.open(filename);
  if (!file_handle_.is_open()) {
    cerr<<"Could not read the raw data file "<<filename<<" in the parser"<<endl;
    return;
  }
  return;
}

bool TRECParser::NextDocument() {
  if (!file_handle_.is_open()) return false;
  cur_document_.terms.clear();
  int state = 0;
  string line;
  while(getline(file_handle_,line) && state >= 0) {
    switch(state) {
      case 0:
        size_t begin_pos,end_pos;
        if (((begin_pos = line.find("<DOCNO>"))!=string::npos) &&
            ((end_pos = line.find("</DOCNO>"))!=string::npos)) {
          begin_pos += strlen("<DOCNO>");
          while (line[begin_pos]==' ') begin_pos++;
          while (line[end_pos-1]==' ') end_pos--;
          cur_document_.doc_name = line.substr(begin_pos,end_pos-begin_pos);
          state = 1;
        }
        break;
      case 1:
        if (line.substr(0,strlen("<TEXT>")) == "<TEXT>") {
          state = 2;
        } else if (line.substr(0,strlen("</DOC>")) == "</DOC>") {
          state = -1;
        }
        break;
      case 2:
        if (line.substr(0,strlen("</TEXT>")) == "</TEXT>") {
          state = 1;
        } else {
          cur_document_.terms.push_back(line);
        }
        break;
      default: {
        state = -1;
      }
    }
  }
  if (state >= 0) return false;
  else return true;
}
  
 
