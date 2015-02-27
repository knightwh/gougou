#pragma once
#include <algorithm>
#include <iostream>
#include "DAM.hpp"

using namespace std;

template<class T>
void mergeSortDAM(DiskAsMemory *theIO,unsigned beginItem,unsigned middleItem,unsigned endItem,unsigned resultItem)
{
	//cout<<"merge item "<<beginItem<<" to "<<middleItem<<" and "<<middleItem<<" to "<<endItem<< "saved at "<<resultItem<<endl;
	unsigned ai=beginItem,bi=middleItem,ci=resultItem;
	unsigned a,b,c;
	DiskItem *DIa,*DIb,*DIc;
	T *bufferA,*bufferB,*bufferC;
	unsigned unitPerItem = theIO->getItemSize()/sizeof(T);
	DIa = theIO->localItem(ai);
	bufferA = (T*)DIa->content;
	a=0;
	DIb = theIO->localItem(bi);
	bufferB = (T*)DIb->content;
	b=0;
	DIc = theIO->localItem(ci);
	bufferC = (T*)DIc->content;
	c=0;
	while(ai<middleItem && bi<endItem)
	{
		if(c>=unitPerItem)
		{
			delete DIc;
			DIc = theIO->localItem(++ci);
			bufferC = (T*)DIc->content;
			c=0;
		}
		if(bufferA[a] <= bufferB[b])
		{
			memmove(bufferC+c++,bufferA+a++,sizeof(T));
			if(a>=unitPerItem)
			{
				if(++ai < middleItem)
				{
					delete DIa;
					DIa = theIO->localItem(ai);
					bufferA = (T*)DIa->content;
					a=0;
				}
			}
		}
		else
		{
			memmove(bufferC+c++,bufferB+b++,sizeof(T));
			if(b>=unitPerItem)
			{
				if(++bi < endItem)
				{
					delete DIb;
					DIb = theIO->localItem(bi);
					bufferB = (T*)DIb->content;
					b=0;
				}
			}
		}
	}
	if(ai>=middleItem)
	{
		if(c!=unitPerItem)
		{
			memmove(bufferC+c,bufferB+b,sizeof(T)*(unitPerItem-b));
			bi++;
		}
		ci++;
		while(bi<endItem) theIO->copyItem(ci++,bi++);
	}
	else
	{
		if(c!=unitPerItem)
		{
			memmove(bufferC+c,bufferA+a,sizeof(T)*(unitPerItem-a));
			ai++;
		}
		ci++;
		while(ai<middleItem) theIO->copyItem(ci++,ai++);
	}
	delete DIa;
	delete DIb;
	delete DIc;
}

template<class T>
void sortWithinItem(T* buffer,unsigned length)
{
	//cout<<"length="<<length<<endl;
	if(length<=1) return;
	unsigned mid = length/2;
	sortWithinItem(buffer,mid);
	sortWithinItem(buffer+mid,length-mid);
	T* workSpace = new T[length];
	unsigned a=0,b=mid,c=0;
	while(a<mid && b<length)
	{
		if(buffer[a]<=buffer[b]) memmove(workSpace+c++,buffer+a++,sizeof(T));
		else memmove(workSpace+c++,buffer+b++,sizeof(T));
	}
	if(a>=mid)
	{
		memmove(buffer,workSpace,sizeof(T)*c);
	}
	else
	{
		memmove(buffer+c,buffer+a,sizeof(T)*(mid-a));
		memmove(buffer,workSpace,sizeof(T)*c);
	}
	delete[] workSpace;
}					

template<class T>
void sortDAM(DiskAsMemory *theIO,unsigned beginItem,unsigned endItem)
{
	if(beginItem>=endItem) return;
	if(endItem>theIO->getItemCounter()) endItem = theIO->getItemCounter();
	unsigned workSize = endItem-beginItem;
	unsigned curCounter  = theIO -> getItemCounter();
	unsigned itemSize = theIO -> getItemSize();
	unsigned i;
	DiskItem *DI;
	//sort within items
	for(i=beginItem;i<endItem;i++)
	{
		DiskItem *DI = theIO->localItem(i);
		T* buffer = (T*)DI->content;
		sortWithinItem(buffer,itemSize/sizeof(T));
		delete DI;
	}
	//open new workSpace
	for(i=0;i<workSize;i++) {DI=theIO->addNewItem();delete DI;}
	//merge operation
	unsigned step = 1;
	bool workSpaceMark=false;
	while(step<workSize)
	{
		for(i=0;i<workSize;i+=step*2)
		{
			unsigned theMiddle = i+step;
			unsigned theEnd = i+step*2;
			if(theMiddle > workSize) break;
			if(theEnd > workSize) theEnd = workSize;
			//cout<<"i="<<i<<"\ttheEnd="<<theEnd<<endl;
			if(!workSpaceMark) mergeSortDAM<T>(theIO,beginItem+i,beginItem+theMiddle,beginItem+theEnd,curCounter+i);
			else mergeSortDAM<T>(theIO,curCounter+i,curCounter+theMiddle,curCounter+theEnd,beginItem+i);
		}
		step*=2;
		workSpaceMark = !workSpaceMark;
	}
	//move back the data
	if(workSpaceMark)
	{
		for(i=0;i<workSize;i++)
		{
			theIO->copyItem(beginItem+i,curCounter+i);
		}
	}
	//delete workspace
	theIO->truncateItem(curCounter);
}	
	
