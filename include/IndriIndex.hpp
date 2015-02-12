#pragma once
#include "indri/Repository.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/CompressedCollection.hpp"
#include <math.h>
#include <vector>
#include <string>

using namespace std;

class IndriPostingList
{
public:
	std::vector<unsigned> docIDs;
	std::vector<unsigned> TFs;
	unsigned DF;
	unsigned total;

	IndriPostingList() {DF=0;total=0;}
	void addItem(unsigned docID,unsigned TF);
};

void IndriPostingList::addItem(unsigned docID,unsigned TF)
{
	docIDs.push_back(docID);
	TFs.push_back(TF);
	DF++;
	total+=TF;
}

class IndriIndexReader
{
public:
	indri::collection::Repository reaper;
	indri::index::Index* theIndex;
	indri::collection::CompressedCollection* collection;

	double docLengthAvg;
	double collectionN;
	unsigned termN;
	unsigned documentN;

	IndriIndexReader(char* indexPath);
	double getDocLengthAvg();
	IndriPostingList* getPostingList(unsigned i);
	unsigned getDocumentLength(unsigned i);
	unsigned getMaxDF();
	string getTerm(unsigned t);
	string getDoc(unsigned d);
	bool checkTerm(unsigned t);
	~IndriIndexReader();
};

IndriIndexReader::IndriIndexReader(char* indexPath)
{
	reaper.openRead(indexPath);
	indri::server::LocalQueryServer local(reaper);
	theIndex=(*(reaper.indexes()))[0];
	indri::thread::ScopedLock(theIndex->iteratorLock());
	collection = reaper.collection();
	
	if(theIndex==NULL)
	{cerr<< "Could not open the index of "<<indexPath<<endl;return;}
	termN=theIndex->uniqueTermCount();
	documentN=theIndex->documentCount();
	getDocLengthAvg();
	collectionN = theIndex->termCount();
}

string IndriIndexReader::getTerm(unsigned t)
{
	return theIndex->term(t);
}

bool IndriIndexReader::checkTerm(unsigned t)
{
	return (reaper.processTerm(theIndex->term(t)) != "");
}

string IndriIndexReader::getDoc(unsigned d)
{
	return collection->retrieveMetadatum(d,"docno");
}

double IndriIndexReader::getDocLengthAvg()
{
	int i;
	docLengthAvg=0;
	for(i=1;i<=theIndex->documentCount();i++)
	{
		docLengthAvg+=theIndex->documentLength(i);
	}
	docLengthAvg/=theIndex->documentCount();
	return docLengthAvg;
}

unsigned IndriIndexReader::getDocumentLength(unsigned i) {return theIndex->documentLength(i);}

IndriPostingList* IndriIndexReader::getPostingList(unsigned i)
{
	indri::index::DocListIterator* diList;
	IndriPostingList* PL = new IndriPostingList();
	
	diList = theIndex->docListIterator(i);
	if(diList!=NULL)
	{
		diList->startIteration();
		indri::index::DocListIterator::DocumentData *diEntry;
		while((diEntry=diList->currentEntry())!=NULL)
		{
			unsigned docID=diEntry->document;
			unsigned TF=diEntry->positions.size();
			PL->addItem(docID,TF);
			diList->nextEntry();
		}
	}
	else {cerr<<"empty indri docList"<<endl;}
	delete(diList);
	return PL;
}

unsigned IndriIndexReader::getMaxDF()
{
	return theIndex->termCount(theIndex->term(1));
}

IndriIndexReader::~IndriIndexReader() 
{
	reaper.close();
}
