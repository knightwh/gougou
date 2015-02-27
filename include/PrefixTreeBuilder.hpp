#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include "DAM.hpp"
#include <fstream>

#ifndef PrefixTreeNodeItemNum
#define PrefixTreeNodeItemNum 7
#endif

#ifndef PrefixTreeBufferSize
#define PrefixTreeBufferSize 1024
#endif

#ifndef PrefixTreeHeadBufferSize
#define PrefixTreeHeadBufferSize (1024*16)
#endif

using namespace std;

class PrefixTreeNode
{
public:
	void initial(unsigned p);
	unsigned parent;
	unsigned value;
	unsigned char mark;
	char chars[PrefixTreeNodeItemNum];
	unsigned children[PrefixTreeNodeItemNum];
};

void PrefixTreeNode::initial(unsigned p)
{
	parent = p;
	value = 0;
	mark = 0;
}

class PrefixTreeBuilder
{
public:
	PrefixTreeBuilder(char *foldPath);
	unsigned lookupTerm(string s);
	unsigned insertTerm(string s);
	unsigned lookupTerm(char *term);
	unsigned insertTerm(char *term);
	unsigned optimize();
	void expandHeadBuffer(unsigned headBufferSize);
	~PrefixTreeBuilder();
private:
	DiskAsMemory *DAM;
	DiskItem* headNode;
	unsigned headBufferSize;
	unsigned *counter;

	unsigned lookupTermID(char *term,DiskItem* const node);
	unsigned insertTermID(char *term,DiskItem* const node);
	DiskItem* expandWideNode(DiskItem* const node);
};

PrefixTreeBuilder::PrefixTreeBuilder(char *foldPath)
{
	//check whether the record exists
	char filename[512];
	sprintf(filename,"%s/summary",foldPath);
	fstream FS(filename);
	if(FS.good())
	{
		FS.close();
		DAM = new DiskAsMemory(foldPath);
		headNode = DAM->localItem(0);
	}
	else
	{
		DAM = new DiskAsMemory(foldPath,sizeof(PrefixTreeNode),PrefixTreeBufferSize);
		headNode = DAM->addNewItem();
		PrefixTreeNode* content = (PrefixTreeNode*)headNode->content;
		content->initial(0);
		//(PrefixTreeNode*)(headNode->content)->initial(0);
	}
	PrefixTreeNode* content = (PrefixTreeNode*)headNode->content;
	counter = &(content->parent);
}

PrefixTreeBuilder::~PrefixTreeBuilder()
{
	delete headNode;
	delete DAM;
}

unsigned PrefixTreeBuilder::lookupTerm(char *term)
{
	if(*term==0) return 0;
	if(strlen(term)>255) term[255]='\0';
	return lookupTermID(term,headNode);
}

unsigned PrefixTreeBuilder::lookupTerm(string s)
{
	char term[256];
	if(s.length()>255) s=s.substr(0,255);
	strcpy(term,s.c_str());
	return lookupTermID(term,headNode);
}

unsigned PrefixTreeBuilder::insertTerm(string s)
{
	char term[256];
	if(s.length()>255) s=s.substr(0,255);
	strcpy(term,s.c_str());
	return insertTermID(term,headNode);
}

unsigned PrefixTreeBuilder::insertTerm(char* term)
{
	if(*term==0) return 0;
	if(strlen(term)>255) term[255]='\0';
	return insertTermID(term,headNode);
}

unsigned PrefixTreeBuilder::lookupTermID(char* term,DiskItem* const node)
{
	PrefixTreeNode* content = (PrefixTreeNode*)node->content;
	if(*term=='\0') return content->value;
	unsigned num = content->mark;
	unsigned i;
	for(i=0;i<num;i++)
	{
		if(content->chars[i]=='\0') //more records
		{
			DiskItem* childNode = DAM->localItem(content->children[i]);
			unsigned ID = lookupTermID(term,childNode);
			delete childNode;
			return ID;
		}
		else if(content->chars[i]==*term) //matched
		{
			DiskItem* childNode = DAM->localItem(content->children[i]);
			unsigned ID = lookupTermID(term+1,childNode);
			delete childNode;
			return ID;
		}
	}
	return 0;
}

unsigned PrefixTreeBuilder::insertTermID(char* term,DiskItem* const node)
{
	PrefixTreeNode* content = (PrefixTreeNode*)node->content;
	if(*term=='\0') 
	{
		if(content->value<=0) content->value = ++*counter;
		return content->value;
	}
			
	unsigned num = content->mark;
	unsigned i;
	for(i=0;i<num;i++)
	{
		if(content->chars[i]=='\0') //more records
		{
			DiskItem* childNode = DAM->localItem(content->children[i]);
			unsigned ID = insertTermID(term,childNode);
			delete childNode;
			return ID;
		}
		else if(content->chars[i]==*term) // matched
		{
			DiskItem* childNode = DAM->localItem(content->children[i]);
			unsigned ID = insertTermID(term+1,childNode);
			delete childNode;
			return ID;
		}
	}
	if(num>=PrefixTreeNodeItemNum) 
	{
		DiskItem* followNode=expandWideNode(node);
		content = (PrefixTreeNode*)followNode->content;
		DiskItem* childNode = DAM->addNewItem();
		PrefixTreeNode* childContent = (PrefixTreeNode*)(childNode->content);
		childContent->initial(followNode->itemNum);
		content->chars[content->mark] = *term;
		content->children[content->mark] = childNode->itemNum;
		content->mark++;
		delete followNode;
		unsigned ID = insertTermID(term+1,childNode);
		delete childNode;
		return ID;
	}
	else
	{
		DiskItem* childNode = DAM->addNewItem();
		PrefixTreeNode* childContent = (PrefixTreeNode*)childNode->content;
		childContent->initial(node->itemNum);
		content->chars[num] = *term;
		content->children[num] = childNode->itemNum;
		content->mark++;
		unsigned ID = insertTermID(term+1,childNode);
		delete childNode;
		return ID;
	}
}

DiskItem* PrefixTreeBuilder::expandWideNode(DiskItem* const node)
{
	DiskItem* followNode = DAM->addNewItem();
	PrefixTreeNode* con = (PrefixTreeNode*)node->content;
	PrefixTreeNode* followCon = (PrefixTreeNode*)followNode->content;
	followCon->initial(node->itemNum);
	followCon->chars[0] = con->chars[PrefixTreeNodeItemNum-1];
	followCon->children[0]= con->children[PrefixTreeNodeItemNum-1];
	con->chars[PrefixTreeNodeItemNum-1] = '\0';
	con->children[PrefixTreeNodeItemNum-1] = followNode->itemNum;
	followCon->mark=1;
	return followNode;
}
	
		
		
		
