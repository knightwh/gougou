#include "VectorInfo.hpp"
#include "VectorInfoReader.hpp"

#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <vector>

#define TEST_SIZE 10000
#define TEST_LENGTH 1000

using namespace std;

int main(int argc, char** argv) {
  srand ( unsigned ( std::time(0) ) );
  vector<vector<unsigned> > res;
  VectorInfo* VI = new VectorInfo("Vector_temp");
  for (unsigned i=1; i<TEST_SIZE; i++) {
    res.resize(i+1);
    unsigned length = rand() % TEST_LENGTH + 1;
    res[i].resize(length);
    for (unsigned l=0; l<length; l++) {
      res[i][l] = rand() % 10000;
    }
    VI->AddVector(res[i],i);
  }
  delete VI;

  // Begin test the reader.
  cout<<"Begin test the reader"<<endl;
  VectorInfoReader* VIR = new VectorInfoReader("Vector_temp");
  for (unsigned i=1; i<TEST_SIZE; i++) {
    //cout<<"vector "<<i<<":"<<endl;
    if (res[i].size() != VIR->GetLength(i)) cout<<i<<":"<<res[i].size()<<" vs "<<VIR->GetLength(i)<<endl;
    vector<unsigned> v = VIR->GetVector(i);
    for (unsigned l=0; l<res[i].size(); l++) {
      if (res[i][l] != v[l]) cout<<i<<":"<<l<<":"<<res[i][l]<<" vs "<<v[l]<<endl;
    }
  }
  
  delete VIR;
  return 0;
}
