#pragma once
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PrefixTreeBlockItemNum 7
#define PrefixTreeDiskBlock (4096*128)

using namespace std;

struct PrefixTreeNode
{
	unsigned parent;
	unsigned count;
	unsigned value;
	char mark;
	char chars[PrefixTreeBlockItemNum];
	unsigned children[PrefixTreeBlockItemNum];
	//PrefixTreeNode(unsigned p) : parent(p),count(0),value(0),mark(0) {}
};	

class PrefixTreeBuilder
{
public:
	PrefixTreeBuilder(char *filename);
	unsigned lookupTerm(char *term);
	unsigned insertTerm(char *term);
	unsigned optimize();
	void finish();
	~PrefixTreeBuilder();
private:
	unsigned nodeSize;
	int fileHandle;
	unsigned* fileUsage;
	unsigned* fileSize;
	unsigned* nodeCounter;
	unsigned bufferUsage;
	char* headBuffer,*tailBuffer,*bodyBuffer;
	PrefixTreeNode *head;

	void expendFileSize();
	inline unsigned addNewNode(unsigned p);
	char* matchTerm(PrefixTreeNode* &node,char* term);
};

PrefixTreeBuilder::PrefixTreeBuilder(char *filename)
{
	nodeSize = sizeof(PrefixTreeNode);
	fileHandle = open(filename,O_RDWR);
	if(fileHandle==-1)
	{
		cerr<<"Could not read old file,Create new file!"<<endl;
		fileHandle = open(filename,O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
		if(fileHandle==-1)
		{
			cerr<<"Could not create new file!"<<endl;
			exit(0);
		}
		lseek(fileHandle, PrefixTreeDiskBlock-1, SEEK_SET);
		if(write(fileHandle,"",1)!=1)
		{
			close(fileHandle);
			cerr<<"Could not expand the file!"<<endl;
			exit(0);
		}
		headBuffer = (char*)mmap(0, PrefixTreeDiskBlock, PROT_READ | PROT_WRITE, MAP_SHARED, fileHandle, 0);
		if(headBuffer == MAP_FAILED)
		{
			close(fileHandle);
			cerr<<"mmap failure!"<<endl;
			exit(0);
		}
		fileSize = (unsigned*)headBuffer;
		fileUsage = fileSize+1;
		nodeCounter = fileSize+2;
		*fileSize = PrefixTreeDiskBlock;
		*fileUsage = sizeof(unsigned)*3;
		tailBuffer = NULL;
		head = (PrefixTreeNode*)(fileSize+3);
		addNewNode(0);
	}
	else
	{
		headBuffer = (char*)mmap(0,PrefixTreeDiskBlock, PROT_READ | PROT_WRITE, MAP_SHARED, fileHandle, 0);
		if(HeadBuffer == MAP_FAILED)
		{
			close(fileHandle);
			cerr<<"mmap failure at the head!"<<endl;
			exit(0);
		}
		fileSize = (unsigned*)headBuffer;
		fileUsage = fileSize+1;
		nodeCounter = fileSize+2;
		head = (PrefixTreeNode*)(fileSize+3);
		if(*fileUsage > PrefixTreeDiskBlock)
		{
			tailBuffer = (char*)mmap(0,PrefixTreeDiskBlock, PROT_READ | PROT_WRITE, MAP_SHARED, fileHandle, *fileUsage - PrefixTreeDiskBlock);
			if(tailBuffer == MAP_FAILED)
			{
				close(fileHandle);
				cerr<<"mmap failure at the tail!"<<endl;
				exit(0);
			}
		}
		else tailBuffer = NULL;
	}
	bufferUsage = *fileUsage % PrefixTreeDiskBlock;
	bodyBuffer = NULL;
}

inline void PrefixTreeBuilder::newNode(PrefixTreeNode* node,unsigned p)
{
	node->parent=p;
	node->count=0;
	node->value=0;
	node->mark=0;
}

unsigned PrefixTreeBuilder::insertTerm(char* term)
{
	if(strlen(term)==0) return 0;
	if(strlen(term)>255) term[255]='\0';
	PrefixTreeNode* node=head;
	char* t=term;
	while(*t!='\0')
	{
		t=matchTerm(node,t);
	}
	if(node->value==0) //new node
	{
		node->value=++*nodeCounter;
		return *nodeCounter;
	}
	else return node->value;
}
		
	
unsigned PrefixTreeBuilder::matchTerm(PrefixTreeNode* &node,char* &term)
{
	if(*term='\0') return term;
	bool type = node->mark & 128;
	unsigned num = node->mark & 127;
	unsigned i,l;
	if(!type)
	{
		for(i=0;i<num;i++)
		{
			if(node->chars[i]==*term) //matched
			{
				node = seekNode(node->children[i]);
				term++;
				return 0;
			}
			if(node->chars[i]=='\0') //more records
			{
				node = seekNode(node->children[i]);
				return 0;
			}
		}
		//no match and add new children
		unsigned newNodeOffset = addNewNode((char*)node - headBuffer);
		if(num<PrefixTreeBlockItemNum) //children are not full
		{
			node->chars[num]=*term;
			node->children[num]=newNodeOffset;
			node->mark[num]++;
		}
		else
		{
			subNodeOffset = addNewNode((char*)node - headBuffer);
			PrefixTreeNode* subNode = seekNode(subNodeOffset);
			subNode->chars[0]=node->chars[num-1];
			subNode->children[0]=node->children[num-1];
			node->chars[num-1]='\0';
			node->children[num-1]=subNodeOffset;
			subNode->chars[1]=*term;
			subNode->children[1]=newNodeOffset;
			subNode->mark[num]=2;
		}
		node = seekNode(newNodeOffset);
		term++;
		return 0;
	}
	for(i=0;i<num;i++)
	{
		if(node->chars[i]=='\0') //more records
		{
			node = seekNode(node->children[i]);
			return 0;
		}
		if(term[i]=='\0')
		{
			if(node->children[i]==0)
			{
				node->children[i]=++*nodeCounter;
				return nodeCounter;
			}
			else return node->children[i];
		}
		if(term[i]!=node->chars[i])
		{
			if(i==0)
			{
				unsigned subNodeOffset = addNewNode((char*)node - headBuffer);
				PrefixTreeNode* subNode = seekNode(subNodeOffset);
				if(num<=2)
				{
					subNode->mark=num-1;
					subNode->chars[0]=node->chars[1];
					subNode->children[0]=node->children[1];
				}
				else
				{
					subNode->mark=num-1;
					subNode->mark |= 128;
					for(l=1;l<num;l++)
					{
						subNode->chars[l-1]=node->chars[l];
						subNode->children[l-1]=node->children[l];
					}
				}
				subNode->value = node->children[0];
				unsigned newNodeOffset = addNewNode((char*)node - headBuffer);
				node->mark = 2;
				node->children[0] = subNodeOffset;
				node->chars[1] = *term;
				node->children[1] = newNodeOffset;
				node = seekNode(newNodeOffset);
				term++;
				return 0;
			}
			else
			{
				unsigned subNodeOffset = addNewNode((char*)node - headBuffer);
				PrefixTreeNode* subNode = seekNode(subNodeOffset);
				if(num-i<=2)
				{
					subNode->mark=num-i-1;
					subNode->chars[0]=node->chars[i+1];
					subNode->children[0]=node->children[i+1];
				}
	
		
			
