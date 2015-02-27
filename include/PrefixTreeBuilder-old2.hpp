#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include "FileIO.hpp"
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

#ifndef PrefixTreeBufferNum
#define PrefixTreeBufferNum 16
#endif

using namespace std;

class PrefixTreeNode
{
public:
	void initial(unsigned p);
	unsigned parent;
	unsigned count;
	unsigned value;
	unsigned char mark;
	char chars[PrefixTreeNodeItemNum];
	unsigned children[PrefixTreeNodeItemNum];
};

void PrefixTreeNode::initial(unsigned p)
{
	parent = p;
	count = 0;
	value = 0;
	mark = 0;
}

class PrefixTreeBuilder
{
public:
	PrefixTreeBuilder(char *foldPath);
	unsigned lookupTerm(char *term);
	unsigned insertTerm(char *term);
	unsigned optimize();
	void expandHeadBuffer();
	~PrefixTreeBuilder();
private:
	FileIO *theIO;
	PrefixTreeNode* headNode;
	unsigned headBufferSize;
	unsigned *counter;
	
	void addNewChild(PrefixTreeNode* node,unsigned nodeNum,unsigned childNum,char theChar);
	unsigned addNewNodes(unsigned preNode,char* str);
	void addNewBranch(PrefixTreeNode* node,unsigned preNode,unsigned theChild,char theChar);
	bool resetChild(unsigned parent,unsigned oldChildNum,unsigned newChildNum);
};

PrefixTreeBuilder::PrefixTreeBuilder(char *foldPath)
{
	//check whether the record exists
	char filename[512];
	unsigned offset;
	sprintf(filename,"%s/summary",foldPath);
	fstream FS(filename);
	if(FS.good())
	{
		FS.close();
		theIO = new FileIO(foldPath,PrefixTreeBufferNum);
		headNode = (PrefixTreeNode*)theIO->localItem(0);
	}
	else 
	{
		theIO = new FileIO(foldPath,sizeof(PrefixTreeNode),PrefixTreeBufferSize,PrefixTreeBufferNum);
		headNode = (PrefixTreeNode*)theIO->addNewItem(offset);
		headNode->initial(0);
	}
	expandHeadBuffer();
}

PrefixTreeBuilder::~PrefixTreeBuilder()
{
	delete(theIO);
}

void PrefixTreeBuilder::expandHeadBuffer()
{
	headBufferSize = theIO->createHeadBuffer(PrefixTreeHeadBufferSize);
	headNode = (PrefixTreeNode*)theIO->localItem(0);
	counter = &headNode->parent;
}

unsigned PrefixTreeBuilder::lookupTerm(char *term)
{
	if(*term==0) return 0;
	if(strlen(term)>255) term[255]='\0';
	PrefixTreeNode* node=headNode;
	while(node!=NULL)
	{
		bool type = node->mark & 128;
		unsigned num = node->mark & 127;
		unsigned i,l;
		if(!type) // wide node
		{
			for(i=0;i<num;i++)
			{
				if(node->chars[i]=='\0') //more records
				{
					node = (PrefixTreeNode*)theIO->localItem(node->children[i]);
					break;
				}
				else if(node->chars[i]==*term) //matched
				{
					node = (PrefixTreeNode*)theIO->localItem(node->children[i]);
					term++;
					if(*term=='\0') return node->value;
					break;
				}
			}
			if(i>=num) return 0;
		}
		else //deep node
		{
			for(i=0;i<num;i++)
			{
				if(node->chars[i]==*term) // matched
				{
					term++;
					if(*term=='\0' || i>=PrefixTreeNodeItemNum)
					{
						if(node->children[i]==0) return 0;
						node = (PrefixTreeNode*)theIO->localItem(node->children[i]);
						if(*term=='\0') return node->value;
					}
				}
				else
				{
					if(i==0) return 0;
					else
					{
						if(node->children[i-1]==0) return 0;
						node = (PrefixTreeNode*)theIO->localItem(node->children[i-1]);
						break;
					}
				}
			}
			if(i>=num) return 0;
		}
	}
	return 0;
}

unsigned PrefixTreeBuilder::insertTerm(char* term)
{
	if(*term==0) return 0;
	if(strlen(term)>255) term[255]='\0';
	PrefixTreeNode* node = headNode;
	unsigned preNode=0;
	while(node!=NULL)
	{
		bool type = node->mark & 128;
		unsigned num = node->mark & 127;
		unsigned i,l;
		if(!type) // wide node
		{
			cout<<"wide node"<<endl;
			for(i=0;i<num;i++)
			{
				if(node->chars[i]=='\0') //more records
				{
					preNode = node->children[i];
					node = (PrefixTreeNode*)theIO->localItem(preNode);
					break;
				}
				else if(node->chars[i]==*term) //matched
				{
					preNode = node->children[i];
					node = (PrefixTreeNode*)theIO->localItem(preNode);
					term++;
					if(*term=='\0') return node->value;
					break;
				}
			}
			if(i>=num)
			{
				unsigned theChild = addNewNodes(preNode,term+1);
				addNewChild(node,preNode,theChild,*term);
				return *counter;
			}
		}
		else //deep node
		{
			cout<<"deep node"<<endl;
			for(i=0;i<num;i++)
			{
				if(node->chars[i]==*term) // matched
				{
					term++;
					if(*term=='\0')
					{
						if(node->children[i]==0) // new record
						{
							unsigned theChild = addNewNodes(preNode,term);
							node->children[i] = theChild;
							return *counter;
						}
						else
						{
							node = (PrefixTreeNode*)theIO->localItem(node->children[i]);
							return node->value;
						}
					}
				}
				else
				{
					if(i==0)
					{
						tranDeepToWide(node,preNode);
						unsigned theChild = addNewNodes(preNode,term+1);
						addNewChild(node,preNode,theChild,*term);
						//addNewBranch(node,preNode,theChild,*term);
						return *counter;
					}
					else
					{
						if(node->children[i-1]==0)
						{
							unsigned theChild = addNewNodes(preNode,term);
							node->children[i-1] = theChild;
							return *counter;
						}
						else
						{
							preNode = node->children[i-1];
							node = (PrefixTreeNode*)theIO->localItem(node->children[i-1]);
							break;
						}
					}
				}
			}
			if(i>=num) 
			{
				if(num>=PrefixTreeItemNum)
				{
					if(node->children[num-1]==0)
					{
						unsigned theChild = addNewNodes(preNode,term);
						node->children[num-1] = theChild;
						return *counter;
					}
					else
					{
						preNode = node->children[num-1];
						node = (PrefixTreeNode*)theIO->localItem(node->children[num-1]);
					}
				}
				else
				{
					while(*term!='\0' && i<PrefixTreeItemNum)
					{
						node->chars[i]=*term;
						node->children[i]=0;
						i++;
						term++;
					}
					unsigned theChild = addNewNodes(preNode,term);
					node->children[i-1]=theChild;
					node->mark = 128 + i;
					return *counter;
				}
			}
		}
	}
	return 0;
}		

void PrefixTreeBuilder::addNewChild(PrefixTreeNode* node,unsigned nodeNum,unsigned childNum,char theChar)
{
	unsigned num = node->mark & 127;
	if(num<PrefixTreeNodeItemNum)
	{
		node->chars[num] = theChar;
		node->children[num] = childNum;
		node->mark++;
	}
	else
	{
		unsigned offset;
		PrefixTreeNode* newNode=(PrefixTreeNode*)theIO->addNewItem(offset);
		newNode->initial(nodeNum);
		newNode->chars[0] = node->chars[PrefixTreeNodeItemNum-1];
		newNode->children[0] = node->children[PrefixTreeNodeItemNum-1];
		node->chars[PrefixTreeNodeItemNum-1] = '\0';
		node->children[PrefixTreeNodeItemNum-1] = offset;
		newNode->chars[1] = theChar;
		newNode->children[1] = childNum;
		newNode->mark = 2;
	}
}

void PrefixTreeBuilder::tranDeepToWide(PrefixTreeNode* node,unsigned nodeNum)
{
	if(node->mark & 128 == 0) return; // wide node
	if(node->mark == 128 || node->mark == 129)
	{
		node->mark = node->mark & 127; //turn the one item deep node to wide node
		return;
	}
	char tempChars[PrefixTreeNodeItemNum];
	unsigned tempChildren[PrefixTreeNodeItemNum];
	unsigned i;
	unsigned num = node->mark & 127;
	for(i=1;i<num;i++)
	{
		tempChars[i-1]=node->chars[i];
		tempChildren[i-1]=node->children[i];
	}
	node->mark = 1;
	while(tempChars[PrefixTreeNodeItemNum-2]=='\0') //has child
	{
		node = 



	unsigned newNodeNum;
	PrefixTreeNode *newNode = (PrefixTreeNode*)theIO->addNewItem(newNodeNum);
	newNode->initial(nodeNum);
	unsigned i;
	unsigned num = node->mark & 127;
	for(i=1;i<num;i++)
	{
		newNode->chars[i-1]=node->chars[i];
		newNode->children[i-1]=node->children[i];
		newNode->mark = 128+num-1;
	}
	if(num==PrefixTreeNodeItem)
	{
		
}

void PrefixTreeBuilder::addNewBranch(PrefixTreeNode* node,unsigned preNode,unsigned theChild,char theChar)
{
	unsigned offset;
	PrefixTreeNode* childNode;
	//PrefixTreeNode* newNode=(PrefixTreeNode*)theIO->addNewItem(offset);
	char headChar = node->chars[0];
	unsigned headChild = node->children[0];
	unsigned num = node->mark & 127;
	unsigned parent = node->parent;
	unsigned curNode = preNode;
	unsigned i;
	while(true)
	{
		for(i=0;i+2<num;i++)
		{
			node->chars[i]=node->chars[i+1];
			node->children[i]=node->children[i+1];
		}
		if(node->chars[num-1]=='\0')
		{
			PrefixTreeNode* newNode=(PrefixTreeNode*)theIO->localItem(node->children[num-1]);
			node->chars[num-2]=newNode->chars[0];
			node->children[num-2]=newNode->children[0];
			childNode = (PrefixTreeNode*)theIO->localItem(node->children[num-2]);
			childNode->parent = curNode;
			curNode = node->children[num-2];
			node = newNode;
			num = node->mark & 127;
		}
		else
		{
			node->chars[num-2]=node->chars[num-1];
			node->children[num-2]=node->children[num-1];
			break;
		}
	}
	if(num<=1) offset = curNode;
	else
	{
		node = (PrefixTreeNode*)theIO->addNewItem(offset);
	}
	resetChild(parent,preNode,offset);
	node->chars[0] = headChar;
	node->children[0] = headChild;
	node->chars[1] = theChar;
	node->chars[1] = theChild;
	node->mark = 2;
	childNode = (PrefixTreeNode*)theIO->localItem(theChild);
	childNode->parent = offset;
}
	
unsigned PrefixTreeBuilder::addNewNodes(unsigned preNode,char* str)
{
	unsigned offset;
	PrefixTreeNode* node = (PrefixTreeNode*)theIO->addNewItem(offset);
	node->initial(preNode);
	if(*str=='\0') // empty node
	{
		node->value = ++*counter;
		return offset;
	}
	if(*(str+1)=='\0') // wide node
	{
		node->chars[0]=*str;
		unsigned theChild = addNewNodes(offset,str+1);
		node->children[0]=theChild;
		node->mark = 1;
		return offset;
	}
	// deep node
	unsigned i=0;
	if(strlen(str)>=PrefixTreeNodeItemNum)
	{
		for(i=0;i<PrefixTreeNodeItemNum-1;i++)
		{
			node->chars[i]=*str;
			node->children[i]=0;
			str++;
		}
		node->mark = 128+PrefixTreeNodeItemNum;
		unsigned theChild = addNewNodes(offset,str);
		node->children[PrefixTreeNodeItemNum-1]=theChild;
		node->chars[PrefixTreeNodeItemNum-1]='\0';
		return offset;
	}
	//short node
	i=0;
	while(*str!='\0')
	{
		node->chars[i]=*str;
		node->children[i]=0;
		i++;
		str++;
	}
	node->mark = 128+i;
	unsigned theChild = addNewNodes(offset,str);
	node->children[i-1]=theChild;
	return offset;
}

bool PrefixTreeBuilder::resetChild(unsigned parent,unsigned oldChildNum,unsigned newChildNum)
{
	PrefixTreeNode *node = (PrefixTreeNode*)theIO->localItem(parent);
	unsigned num = node->mark & 127;
	unsigned i;
	for(i=0;i<num;i++)
	{
		if(node->children[i]==oldChildNum)
		{
			node->children[i]=newChildNum;
			return true;
		}
	}
	return false;
}
