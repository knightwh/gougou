#include "IndexBuilder.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

void IndexBuilder::ProcessFile(char* filename) {
  ifstream F1;
  F1.open(filename);
  if(!F1) {
    cerr << "Error: parameter file "<<filename<<" could not be read"<<endl;
    return;
  }
  string line;
  while(getline(F1,line)) {
    if (line.substr(0,strlen("IndexPath:")) == "IndexPath:") {
      if (!index_path_.empty()) {
        cerr << "Repeat define index path"<<endl;
      }
      index_path_ = RemoveSpace(line.substr(strlen("IndexPath:")));
    } else if(line.substr(0,strlen("DataPath:")) == "DataPath:") {
      data_path_.push_back(RemoveSpace(line.substr(strlen("DataPath:"))));
    }
  }
  F1.close();
}

string IndexBuilder::RemoveSpace(const string& str) {
  string res;
  for(unsigned i=0; i<str.length(); i++) {
    if (str[i]!=' ' && str[i]!='\t' && str[i]!='\n') {
      res += str[i];
    }
  }
  return res;
}

void IndexBuilder::Show(ostream& is) {
  is<<"IndexPath:"<<index_path_<<endl;
  is<<"DataPath:"<<endl;
  for(unsigned i=0; i<data_path_.size(); i++) {
    is<<data_path_[i]<<endl;
  }
}

void IndexBuilder::BuildIndex() {
  if(index_path_.empty()) {
    cerr<<"You should define the index path"<<endl;
    return;
  }
  if(data_path_.empty()) {
    cerr<<"You should specific at least one data path"<<endl;
    return;
  }
  char index_path[256];
  char file_path[256];
  strcpy(index_path,index_path_.c_str());
  IndexBuilderBase* IBB = new IndexBuilderBase(index_path);
  for(unsigned i=0; i<data_path_.size(); i++) {
    struct stat buf;
    string path = data_path_[i];
    if( lstat(path.c_str(), &buf)==-1) {
      cerr<<"Invalid data path:"<<path<<endl;
    } else if (S_ISREG(buf.st_mode)) {
      // text file.
      strcpy(file_path,path.c_str());
      cout<<"Open file "<<file_path<<endl;
      IBB->BuildIndex(file_path);
    } else {
      // fold.
      DIR *dp;
      struct dirent *dirp;
      if((dp  = opendir(path.c_str())) == NULL) {
        cerr << "Could not open the fold" << path << endl;
        break;
      }
      while ((dirp = readdir(dp)) != NULL) {
        string filename = (string)dirp->d_name;
        if(!filename.empty() && filename[0]!='.') {
          filename = path + '/' + filename;
          strcpy(file_path,filename.c_str());
          cout<<"Open file "<<file_path<<endl;
          IBB->BuildIndex(file_path);
        }
      }
      closedir(dp);
    }
  }
  IBB->Finalize();
  delete IBB;
}
