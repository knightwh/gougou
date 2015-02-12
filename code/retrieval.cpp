#include "query.hpp"
#include <iostream>
#include <math.h>
#include "IndexReaderBase.hpp"
#include "MinHeap.hpp"
#include <string>
#include <fstream>
#include "TimeCounter.hpp"
#include <stdlib.h>

using namespace std;

#define MaxNum 0xFFFFFFFF

int RetNum=1000;

struct termReter
{
	unsigned curDocID;
	PostingListReader *PR;
	unsigned termID;
	float QF;
	float maxScore;
};

class RetManager
{
public:
	RetManager(IndexReaderBase *IR,query *q);
	void retrieval(unsigned retNum);
	void show(ostream& FO);
	~RetManager();
private:
	IndexReaderBase *theIndex;
	unsigned num;
	string topicNum;

	termReter *r;
	
	unsigned* retDocID;
	float* retDocScore;
	unsigned retN;
	unsigned curDoc;
	MinHeap* topDocs;


	inline unsigned findNextDoc();
	inline float grade();
};

RetManager::RetManager(IndexReaderBase *IR,query *q)
{
	unsigned i;
	theIndex = IR;
	num = q->term.size();
	topicNum = q->topicNum;
	// Initial arrays
	r = new termReter[num];
	// set initial values
	for(i=0;i<num;i++)
	{
		r[i].QF = q->tf[i];
		r[i].termID = theIndex->TermLookup(q->term[i]);
		if(r[i].termID!=0)
		{
			r[i].PR = theIndex->GetPosting(r[i].termID);
			if(r[i].PR!=NULL && r[i].PR->NextDoc()) r[i].curDocID = r[i].PR->CurDocID();
			else {r[i].curDocID = MaxNum;cout<<"Could not find the term "<<q->term[i]<<endl;}
			//r[i].maxScore = theIndex->getMaxScore(r[i].termID);
			//cout<<"maxScore="<<r[i].maxScore<<endl;
		}
		else
		{
			r[i].PR = NULL;
			r[i].curDocID = MaxNum;
			//r[i].maxScore = 0;
		}
	}
	// set other values
	retDocID = NULL;
	retDocScore = NULL;
	retN = 0;
	curDoc = 0;
	topDocs = NULL;
}

RetManager::~RetManager()
{
	unsigned i;
	for(i=0;i<num;i++) if(r[i].PR!=NULL) delete r[i].PR;
	delete[] r;
	if(retDocID != NULL) delete[] retDocID;
	if(retDocScore!=NULL) delete[] retDocScore;
}

void RetManager::retrieval(unsigned retNum)
{
	retDocID = new unsigned[retNum];
	retDocScore = new float[retNum];
	topDocs = new MinHeap(retNum);
	
	while((curDoc = findNextDoc())!=MaxNum)
	{
    //float score = 0;
		float score = grade();
		if(score > topDocs->smallest) topDocs->push(curDoc,score);
	}
	retN = topDocs->n;
	int i;
	for(i=retN-1;i>=0;i--)
	{
		retDocID[i] = topDocs->pop(retDocScore[i]);
	}
	delete(topDocs);
}

void RetManager::show(ostream &OF)
{
	unsigned i;
	for(i=0;i<retN;i++)
	{
		OF<<topicNum<<"\t";
		OF<<theIndex->DocName(retDocID[i])<<"\t";
		//OF<<retDocID[i]<<"\t";
		OF<<retDocScore[i]<<endl;
	}
}

unsigned inline RetManager::findNextDoc()
{
	unsigned minDoc = MaxNum;
	unsigned i,l;
	float s=0.0f;

	for(i=0;i<num;i++)
	{
		if(r[i].curDocID<=curDoc) 
		{
			if(r[i].PR->NextDoc()) r[i].curDocID = r[i].PR->CurDocID();
			else r[i].curDocID = MaxNum;
		}
	}
	for(i=0;i<num;i++)
	{
		if(minDoc> r[i].curDocID) minDoc = r[i].curDocID;
	}
	return minDoc;
}

float inline RetManager::grade()
{
	const float okapiB = 0.2;
	const float okapiK1 = 1.2;
	const float  okapiK3 = 7;
	float score = 0;
	float docN = theIndex -> DocCount();
	float docLength = theIndex -> DocLength(curDoc);
	//float docLength = 100;
	float docLengthAvg = theIndex -> DocLengthAvg();
	unsigned i;
	for(i=0;i<num;i++)
	{
		if(curDoc == r[i].curDocID)
		{
			float DF = r[i].PR->GetSummary().df;
			float tf = r[i].PR->CurTF();
			float idf = log((docN-DF+0.5)/(DF+0.5));
			//float idf = log((1.0+docN)/DF);
			//float weight = (1.0+log(1+log(tf)))/(1.0-okapiB+okapiB*docLength/docLengthAvg);
			float weight = ((okapiK1+1.0)*tf) / (okapiK1*(1.0-okapiB+okapiB*docLength/docLengthAvg)+tf);
			float tWeight = ((okapiK3+1)*r[i].QF)/(okapiK3+r[i].QF);
			score+=idf*weight*tWeight;
			//if(curDoc == 525647) cout<<"i="<<i<<":"<<tf<<endl;
			//score+=idf*weight;
		}
	}
	return score;
}
	
void retrieval(char *indexPath,queryManager *queries,char *outputFile)
{
	IndexReaderBase *theIndex = new IndexReaderBase(indexPath);
	//char blockInfoName[] = "BM25-02";
	//theIndex -> loadBlockInfo(blockInfoName);
	
	ofstream F1;
	F1.open(outputFile);
	if(!F1) {cerr << "Error: file "<<outputFile<<" Could not be writ to"<<endl; return;}
	
	TimeCounter *timer = new TimeCounter();
	unsigned i;
	for(i=0;i<queries->num();i++)
	{
		timer->start();
		query* qry = queries -> getQuery(i);
		RetManager *reter = new RetManager(theIndex,qry);
		reter->retrieval(RetNum);
		timer->stop();
		reter->show(F1);
		cout<<"\r retrieval topic "<<qry->topicNum<<flush;
		delete reter;
	}
	cout<<endl;
	cout<<"Used total time "<<timer->getTotalTime()<<"ms, average "<<timer->getAverageTime()<<"ms"<<endl;
	F1.close();
	delete(theIndex);
	delete(timer);
}

int main(int argc,char **argv)
{
	if(argc!=4 && argc!=5) { cout<<"Usage: "<<argv[0]<<" indexPath qry outputFile (retNum)"<<endl; return 0;}
	if(argc==5) RetNum=atoi(argv[4]);
	queryManager *queries = new queryManager(argv[2]);
	queries->show();
	retrieval(argv[1],queries,argv[3]);
	delete queries;
	return 0;
}
