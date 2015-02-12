#ifndef query_H
#define query_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class query
{
public:
	std::string topicNum;
	std::vector<std::string> term;
	std::vector<double> tf;
	
	virtual void show();
	virtual unsigned num() {return term.size();}
	virtual void clear();
	virtual void inputQry(char** strs,int l);
};

void query::show()
{
	unsigned i;
	std::cout<<"topic "<<topicNum<<std::endl;
	for(i=0;i<term.size();i++)
	{
		std::cout<<term[i]<<' ';
	}
	std::cout<<std::endl;
}

void query::clear()
{
	term.clear();
	tf.clear();
}

void query::inputQry(char** strs,int l)
{
	int i;
	std::vector<std::string>::iterator it;
	for(i=0;i<l;i++)
	{
		std::ostringstream ostr;
		ostr<<strs[i];
		std::string str=ostr.str();
		for(it=term.begin();it!=term.end();it++)
		{
			if(str==*it) {tf[it-term.begin()]++;break;}
		}
		if(it==term.end())
		{
			term.push_back(str);
			tf.push_back(1);
		}
	}
}		

class queryManager
{
protected:
	std::vector<query> queries;

	virtual void readQueryFile(char *filename);
public:
	queryManager(char* filename) {readQueryFile(filename);}
	query* getQuery(unsigned n);
	unsigned num() {return queries.size();}
	void show();
};

void queryManager::readQueryFile(char* filename)
{
	ifstream F1;
	std::string topic;
	std::string line;
	int pc,bc;
	query theQuery;
	
	F1.open(filename);
	if(!F1) { // file couldn't be opened
		std::cerr << "Error: file"<<filename<<" could not be opened"<<std::endl;
		return;
	}

	while(!F1.eof())
	{
		std::getline(F1,line);
		if(line.size()<=0) continue;
		if((pc=line.find("<DOC "))!=std::string::npos)
		{
			pc+=5;
			bc=pc;
			while(line[pc]!='>') pc++;
			theQuery.topicNum=line.substr(bc,pc-bc);
		}
		else if(line.find("</DOC>")!=std::string::npos)
		{
			queries.push_back(theQuery);
			theQuery.clear();
		}
		else
		{
			pc=0;
			while(line[pc]==' ' || line[pc]=='\t' || line[pc]=='\0') pc++;
			if(line[pc]=='\n') continue;
			bc=pc;
			while(line[pc]!=' ' && line[pc]!='\t' && line[pc]!='\n' && line[pc]!='\0') pc++;
			std::string s=line.substr(bc,pc-bc);
			std::vector<std::string>::iterator it;
			for(it=theQuery.term.begin();it!=theQuery.term.end();it++)
			{
				if(s==*it) {theQuery.tf[it-theQuery.term.begin()]++;break;}
			}
			if(it==theQuery.term.end())
			{
				theQuery.term.push_back(s);
				theQuery.tf.push_back(1);
			}
		}
	}
	F1.close();
}

query* queryManager::getQuery(unsigned n) 
{
	if(n>=queries.size()) return NULL;
	else return &queries[n];
}

void queryManager::show()
{
	unsigned i;
	for(i=0;i<queries.size();i++) queries[i].show();
}


#endif
