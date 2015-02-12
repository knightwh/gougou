#include "DAM.hpp"
#include <climits>

using namespace std;

DiskAsMemory::DiskAsMemory(char *foldname,unsigned iS,unsigned bS)
{
	mkdir(foldname,S_IRWXU);
	char filename[512];
	unsigned i;
	itemSize=iS;
	specialHead = NULL;
	bufferSize = bS;
	if(!validPara()) return;
	strcpy(foldPath,foldname);
	sprintf(filename,"%s/summary",foldPath);
	fileHandleSummary = open(filename,O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
	if(fileHandleSummary==-1)
	{
		cerr<<"Could not create the new file!"<<endl;
		return;
	}
	lseek(fileHandleSummary,sizeof(uint64_t)*2,SEEK_SET);
	write(fileHandleSummary,&itemSize,sizeof(unsigned));
	write(fileHandleSummary,&bufferSize,sizeof(unsigned));
	metaBuffer = (char*)mmap(0,sizeof(unsigned)*2+sizeof(uint64_t)*2,PROT_READ | PROT_WRITE, MAP_SHARED, fileHandleSummary,0);
	if(metaBuffer == MAP_FAILED)
  {
    close(fileHandleSummary);
                cerr<<"mmap failure at the metafile!"<<endl;
                return;
        }
        itemCounter = (uint64_t*)metaBuffer;
        lastFile = -1;
        allocatedSpace = itemCounter+1;
        *itemCounter=0;
        *allocatedSpace=0;
        for(i=0;i<256;i++) fileHandle[i]=-1;
        expandSize();
}

DiskAsMemory::DiskAsMemory(char *foldname)
{
        char filename[512];
        unsigned i;
        strcpy(foldPath,foldname);
        sprintf(filename,"%s/summary",foldPath);
        fileHandleSummary = open(filename,O_RDWR);
        if(fileHandleSummary==-1)
        {
                cerr<<"Could not read the old file!"<<endl;
                return;
        }
        metaBuffer = (char*)mmap(0,sizeof(unsigned)*2+sizeof(uint64_t)*2,PROT_READ | PROT_WRITE,MAP_SHARED,fileHandleSummary,0);
        if(metaBuffer == MAP_FAILED)
        {
                close(fileHandleSummary);
                cerr<<"mmap failure at the metafile!"<<endl;
                return;
        }
        itemCounter = (uint64_t*)metaBuffer;
        allocatedSpace = itemCounter+1; 
        itemSize = *(unsigned*)(metaBuffer+sizeof(uint64_t)*2);
        bufferSize = *(unsigned*)(metaBuffer+sizeof(uint64_t)*2+sizeof(unsigned));
        validPara();
	      specialHead = NULL;
        for(i=0;i<256;i++) fileHandle[i]=-1;
	lastFile = (*allocatedSpace/bufferSize)/bufferPerFile;
	DiskItem *DI = localItem(*allocatedSpace-bufferSize);
	delete DI;
}

DiskAsMemory::~DiskAsMemory()
{
	if(specialHead != NULL) delete specialHead;
	vector<DiskBuffer*>::iterator it;
	for(it=bodys.begin();it!=bodys.end();it++)
	{
		delete *it;
	}
	unsigned i;
	for(i=0;i<=lastFile;i++)
	{
		if(fileHandle[i]!=-1) close(fileHandle[i]);
	}
	munmap(metaBuffer,sizeof(unsigned)*2+sizeof(uint64_t)*2);
	close(fileHandleSummary);
}

bool DiskAsMemory::validPara()
{
	bufferSpace = itemSize * bufferSize;
	unsigned pageSize = sysconf(_SC_PAGESIZE);
	if(bufferSpace % pageSize != 0)
        {
                cerr<<"The buffer size should be the integer times page size of "<<pageSize<<endl;
                return false;
        }
        bufferPerFile = MaxFileSize / bufferSpace;
        return true;
}

DiskItem* DiskAsMemory::addNewItem()
{
	if(*itemCounter>=*allocatedSpace) expandSize();
	return bodys.back()->localItem((*itemCounter)++);
}

DiskItem* DiskAsMemory::localItem(uint64_t itemNum)
{
	DiskItem *DI;
	if(itemNum>*itemCounter) 
	{
		cerr<<"item number overflow"<<itemNum<<" vs "<<*itemCounter<<endl;
		return NULL;
	}
	if(specialHead!=NULL && (DI=specialHead->localItem(itemNum))!=NULL) return DI;
	unsigned searchBegin=0,searchEnd=bodys.size(),searchMid;
	
	if(!bodys.empty()) while(searchBegin<searchEnd)
	{
		searchMid = (searchBegin+searchEnd)/2;
		if((DI=bodys[searchMid]->localItem(itemNum))!=NULL) return DI;
		if(itemNum >= bodys[searchMid]->end) searchBegin = searchMid+1;
		else searchEnd = searchMid;
	}
	//no buffer is found and new buffer needed
	vector<DiskBuffer*>::iterator it;
	unsigned theBufferNum = itemNum / bufferSize;
	unsigned theFileNum = theBufferNum / bufferPerFile;
	unsigned theFilePos = theBufferNum % bufferPerFile * bufferSpace;
	if(fileHandle[theFileNum]==-1)
        {
                char filename[256];
                sprintf(filename,"%s/%d",foldPath,theFileNum);
                fileHandle[theFileNum] = open(filename,O_RDWR);
                if(fileHandle[theFileNum]==-1)
                {
                        cerr<<"Could not read the old file! "<<theFileNum<<endl;
                        return NULL;
                }
        }
	DiskBuffer* DB = new DiskBuffer(this,theBufferNum*bufferSize,theBufferNum*bufferSize+bufferSize,fileHandle[theFileNum],theFilePos);
	bodys.insert(bodys.begin()+searchBegin,DB);
	DI = DB->localItem(itemNum);
	return DI;
}

void DiskAsMemory::expandSize()
{
	unsigned theFileNum = (*allocatedSpace/bufferSize)/bufferPerFile;
	unsigned theFilePos = (*allocatedSpace/bufferSize)%bufferPerFile*bufferSpace;
	if(lastFile<0 || theFileNum>lastFile)
	{
		lastFile=theFileNum;
		char filename[256];
    sprintf(filename,"%s/%d",foldPath,lastFile);
    fileHandle[lastFile] = open(filename,O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
    if(fileHandle[lastFile]==-1)
    {
      cerr<<"Could not create the new file "<<lastFile<<endl;
      return;
    }
  }
	lseek(fileHandle[lastFile],theFilePos+bufferSpace-1,SEEK_SET);
  write(fileHandle[lastFile],"",1);
	DiskBuffer* DB = new DiskBuffer(this,*allocatedSpace,*allocatedSpace+bufferSize,fileHandle[lastFile],theFilePos);
	bodys.push_back(DB);
	*allocatedSpace+=bufferSize;
}

unsigned DiskAsMemory::createHeadBuffer(unsigned headBufferSize)
{
	if(headBufferSize > *allocatedSpace) headBufferSize=*allocatedSpace;
	if(headBufferSize > bufferPerFile*bufferSize) headBufferSize=bufferPerFile*bufferSize;
	if(specialHead!=NULL)
	{
		if(specialHead->getUsage()!=0)
		{
			cerr<<"Unsafe head buffer replacement: someone is still using the old buffer"<<endl;
			return specialHead->end;
		}
		delete specialHead;
	}
	specialHead = new DiskBuffer(this,ULLONG_MAX,headBufferSize,0,0);
	return headBufferSize;
}

bool DiskAsMemory::truncateItem(uint64_t counter)
{
	if(counter>=*allocatedSpace) return false;
	unsigned allocated = (counter+bufferSize-1)/bufferSize*bufferSize;
	if(allocated<bufferSize) allocated = bufferSize;
	if(allocated<*allocatedSpace)
	{
		if(specialHead!=NULL && specialHead->end>allocated)
		{
			if(createHeadBuffer(allocated)>allocated)
			{
				cerr<<"truncate Item Failed because the head buffer is not cleaned!"<<endl;
				return false;
			}
		}

		while(!bodys.empty())
		{
			DiskBuffer *DB=bodys.back();
			if(DB->begin >=counter)
			{
				if(DB->getUsage() > 0)
				{
					cerr<<"truncate Item Failed because the body buffer is not cleaned!"<<endl;
					return false;
				}
				else {delete DB;bodys.pop_back();}
			}
			else break;
		}
		DiskItem* DI = localItem(allocated-1);
		delete DI;
				
		unsigned theFileNum = (allocated/bufferSize)/bufferPerFile;
		unsigned theFilePos = (allocated/bufferSize)%bufferPerFile*bufferSpace;
		//if(theFilePos == 0) theFilePos = bufferPerFile*bufferSpace;
		while(lastFile > theFileNum)
                {
                        if(fileHandle[lastFile]!=-1) close(fileHandle[lastFile]);
                        char filename[256];
                        sprintf(filename,"%s/%d",foldPath,lastFile);
                        unlink(filename);
                        lastFile--;
                }
                ftruncate(fileHandle[theFileNum],theFilePos);
		*allocatedSpace = allocated;
	}
	*itemCounter = counter;
	return true;
}
		
bool DiskAsMemory::freeBuffer(DiskBuffer *DB)
{
	vector<DiskBuffer*>::iterator it;
	for(it=bodys.begin();it!=bodys.end() && it!=bodys.end()-1;it++)
	{
		if(*it==DB)
		{
			delete DB;
			bodys.erase(it);
			return true;
		}
	}
	return false;
}

bool DiskAsMemory::copyItem(uint64_t destination,uint64_t source)
{
	if(source >= *itemCounter || destination >= *itemCounter) return false;
	DiskItem *itemSource = localItem(source);
	DiskItem *itemDestination = localItem(destination);
	memmove(itemDestination->content,itemSource->content,itemSize);
	delete itemDestination;
	delete itemSource;
	return true;
}

bool DiskAsMemory::switchItem(uint64_t A,uint64_t B)
{
	if(A >= *itemCounter || B>=*itemCounter) return false;
	char *temp = new char(itemSize);
	DiskItem *itemA = localItem(A);
	DiskItem *itemB = localItem(B);
	memmove(temp,itemA,itemSize);
	memmove(itemA,itemB,itemSize);
	memmove(itemB,temp,itemSize);
	delete[] temp;
	return true;
}

DiskMultiItem* DiskAsMemory::AddMultiNewItem(unsigned itemNum) {
  while(*itemCounter+itemNum > *allocatedSpace) expandSize();
  *itemCounter += itemNum;
  return LocalMultiItem(*itemCounter-itemNum, itemNum);
}

DiskMultiItem* DiskAsMemory::LocalMultiItem(uint64_t beginNum,unsigned itemNum) {
  if (beginNum+itemNum>*itemCounter || itemNum==0) {
    cerr<<"item number overflow "<<beginNum+itemNum<<" vs "<<*itemCounter<<endl;
    return NULL;
  }
  DiskMultiItem* multi_item = new DiskMultiItem(*itemCounter,itemSize);
  if(specialHead!=NULL && specialHead->end > beginNum) {
    unsigned cnum = itemNum;
    if (cnum > specialHead->end-beginNum) cnum = specialHead->end-beginNum;
    specialHead->PutMultiItems(multi_item,beginNum,cnum);
    itemNum -= cnum;
    beginNum += cnum;
  }
  while(itemNum > 0) {
    unsigned searchBegin=0,searchEnd=bodys.size(),searchMid;
    DiskBuffer* cur_DB = NULL;
    if(!bodys.empty()) {
      while(searchBegin<searchEnd) {
        searchMid = (searchBegin+searchEnd)/2;
        if (bodys[searchMid]->end > beginNum) {
          if (bodys[searchMid]->begin <= beginNum) {
            cur_DB = bodys[searchMid];
            break;
          } else {
            searchEnd = searchMid;
          }
        } else searchBegin = searchMid+1;
      }
    }
    if (cur_DB == NULL) { // no buffer is found and new buffer needed.
      unsigned theBufferNum = beginNum / bufferSize;
      unsigned theFileNum = theBufferNum / bufferPerFile;
      unsigned theFilePos = theBufferNum % bufferPerFile * bufferSpace;
      if(fileHandle[theFileNum]==-1) {
        char filename[256];
        sprintf(filename,"%s/%d",foldPath,theFileNum);
        fileHandle[theFileNum] = open(filename,O_RDWR);
        if(fileHandle[theFileNum]==-1) {
          cerr<<"Could not read the old file! "<<theFileNum<<endl;
          return NULL;
        }
      }
      DiskBuffer* DB = new DiskBuffer(this,theBufferNum*bufferSize,theBufferNum*bufferSize+bufferSize,fileHandle[theFileNum],theFilePos);
      bodys.insert(bodys.begin()+searchBegin,DB);
      cur_DB = DB;
    }
    unsigned cnum = itemNum;
    if (cnum > cur_DB->end-beginNum) cnum = cur_DB->end-beginNum;
    cur_DB->PutMultiItems(multi_item,beginNum,cnum);
    itemNum -= cnum;
    beginNum += cnum;
  }
  return multi_item;
}

