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
	unsigned num = content->mark & 128;
	bool type = content->mark & 127;
	unsigned i;
	if(!type) // wide node
	{
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
	}
	else // deep node
	{
		for(i=0;i<num;i++)
		{
			if(*term=='\0' || content->chars[i]!=*term)
			{
				if(node->children[i]==0) return 0;
				DiskItem* childNode = DAM->localItem(content->children[i]);
				unsigned ID = lookupTermID(term,childNode);
				delete childNode;
				return ID;
			}
			else term++;
		}
		return 0;
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
			
	unsigned num = content->mark & 127;
	bool type = content & 128;
	unsigned i;
	if(!type) // wide node
	{
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
			unsigned ID=0;
			DiskItem* followNode=expandWideNode(node);
			content = (PrefixTreeNode*)followNode->content;
			DiskItem* childNode = addNewTerm(term+1,followNode->itemNum,ID);
			content->chars[content->mark] = *term;
			content->children[content->mark] = childNode->itemNum;
			content->mark++;
			delete followNode;
			delete childNode;
			return ID;
		}
		else
		{
			unsigned ID=0;
			DiskItem* childNode = addNewTerm(term+1,node->itemNum,ID);
			content->chars[num] = *term;
			content->children[num] = childNode->itemNum;
			content->mark++;
			delete childNode;
			return ID;
		}
	}
	else // deep node
	{
		for(i=0;i<num;i++)
		{
			if(*term=='\0' || content->chars[i]!=*term)
			{
				if(node->children[i]==0)
				{
					unsigned ID=0;
					DiskItem* childNode = addNewTerm(term,node->itemNum,ID);
					content->children[i]=childNode->itemNum;
					delete childNode;
					return ID;
				}
				// already has child
				DiskItem* childNode = localItem(content->children[i]);
				if(i==0 && *term!='\0')
				{
					TurnToWide(childNode);
				}
				unsigned ID=insertTermID(term,childNode);
				delete childNode;
				return ID;
			}
			else term++;
		}
		// longer string
		while(*term!='\0')
		{
			if(i>=PrefixTreeNodeItemNum-1) break;
			content->chars[i] = *term;
			content->children[i]=0;
			term++;
		}
		unsigned ID;
		DiskItem* childNode = addNewTerm(term,node->itemNum,ID);
		content->chars[i] = '\0';
		content->children[i] = childNode->itemNum;
		content->mark = 128 + i+1;
		delete childNode;
		return ID;
	}
	return 0;				
}

DiskItem* PrefixTreeBuilder::addNewTerm(char *term,unsigned parent,unsigned &ID)
{
	DiskItem* node = DAM->addNewItem();
	PrefixTreeNode* content = (PrefixTreeNode*)node->content;
	content->initial(parent);
	if(*term=='\0') //empty string
	{
		content->value = ++*counter;
		ID = *counter;
		return node;
	}
	if(*(term+1)=='\0') // one char
	{
		DiskItem* childNode = DAM->addNewItem();
		PrefixTreeNode* childContent = (PrefixTreeNode*)childContent->content;
		childContent->initial(node->itemNum);
		childContent->value = ++*counter;
		ID = *counter;
		content->chars[0] = *term;
		content->children[0] = childNode->itemNum;
		content->mark = 1;
		delete childNode;
		return node;
	}
	// longer string
	unsigned num=0;
	while(*term!='\0')
	{
		if(num>=PrefixTreeItemNum-1) break;
		content->chars[num] = *term;
		content->children[num] = 0;
		term++;
		num++;
	}
	DiskItem* childNode = addNewTerm(term,node->itemNum,ID);
	content->chars[num] = '\0';
	content->children[num] = childNode->itemNum;
	delete childNode;
	content->mark = 128 + num+1;
	return node;
}

void PrefixTreeBuilder::turnToWide(DiskItem *node)
{
	PrefixTreeNode* content = (PrefixTreeNode*)node->content;
	unsigned num = content->mark & 127;
	bool type = content->mark & 128;
	if(!type) return; // wide node already
	if(num==0 || num==1) {content->mark &= 127;return;}
	if(content->children[0]!=0) {cerr<<"strange error at the turnToWide function!"<<endl;return;}
	chars[PrefixTreeNodeItemNum] tempChars;
	unsigned[PrefixTreeNodeItemNum] tempChildren;
	unsigned i;
	for(i=1;i<num;i++)
	{
		tempChars[i-1] = content->chars[i];
		tempChildren[i-1] = content->children[i];
	}
	if(num<PrefixTreeNodeItemNum)
	{	
		DiskItem* childNode = addNewItem();
		PrefixTreeNode* childContent = (PrefixTreeNode*)childNode->content;
		childContent->initial(node->itemNum);
		addChildrenDeep(childNode,tempChars,tempChildren,num-1);
		content->children[0] = childNode->itemNum;
		delete childNode;
	}
	else
	{
		DiskItem* childNode = localItem(content->children[PrefixTreeNodeItemNum-1]);
		addChildrenDeep(childNode,tempChars,tempChildren,num-1);
		content->children[0] = childNode->itemNum;
		delete childNode;
	}
	content->mark = 1;
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
	
		
		
		
