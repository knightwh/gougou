#include "DAMreader.hpp"
#include <climits>

using namespace std;

DiskAsMemoryReader::DiskAsMemoryReader(char *foldname)
{
  char filename[512];
  unsigned i;
  strcpy(foldPath,foldname);
  sprintf(filename,"%s/summary",foldPath);
  fileHandleSummary = open(filename,O_RDONLY);
  if(fileHandleSummary==-1)
  {
    cerr<<"Could not read the old file!"<<endl;
    return;
  }
  metaBuffer = (char*)mmap(0,sizeof(unsigned)*2+sizeof(uint64_t)*2,PROT_READ,MAP_SHARED,fileHandleSummary,0);
  if(metaBuffer == MAP_FAILED)
  {
    close(fileHandleSummary);
    cerr<<"mmap failure at the metafile!"<<endl;
    return;
  }
  itemCounter = (uint64_t*)metaBuffer;
  allocatedSpace = itemCounter+1;
  itemSize = *(unsigned*)(metaBuffer+2*sizeof(uint64_t));
  bufferSize = *(unsigned*)(metaBuffer+2*sizeof(uint64_t)+sizeof(unsigned));
  validPara();
	specialHead = NULL;
  for(i=0;i<256;i++) fileHandle[i]=-1;
	lastFile = (*allocatedSpace/bufferSize)/bufferPerFile;
	DiskItemReader *DI = localItem(*allocatedSpace-bufferSize);
	delete DI;
}

DiskAsMemoryReader::~DiskAsMemoryReader()
{
	if(specialHead != NULL) delete specialHead;
	vector<DiskBufferReader*>::iterator it;
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

bool DiskAsMemoryReader::validPara()
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

DiskItemReader* DiskAsMemoryReader::localItem(uint64_t itemNum)
{
	DiskItemReader *DI;
	if(itemNum>*itemCounter) 
	{
		cerr<<"item number overflow"<<itemNum<<" vs "<<*itemCounter<<endl;
		return NULL;
	}
	if(specialHead!=NULL && (DI=specialHead->localItem(itemNum))!=NULL) return DI;
  unsigned searchBegin=bodys.size();
  if (!bodys.empty()) {
    DiskBufferReader* tail = bodys.back();
    if (itemNum < tail->end) {
      if ((DI=tail->localItem(itemNum))!=NULL) return DI;
      searchBegin = 0;
      unsigned searchEnd=bodys.size(),searchMid;
	    if(!bodys.empty()) while(searchBegin<searchEnd)
	    {
		    searchMid = (searchBegin+searchEnd)/2;
		    if((DI=bodys[searchMid]->localItem(itemNum))!=NULL) return DI;
		    if(itemNum >= bodys[searchMid]->end) searchBegin = searchMid+1;
		    else searchEnd = searchMid;
	    }
    }
  }
	//no buffer is found and new buffer needed
	vector<DiskBufferReader*>::iterator it;
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
	DiskBufferReader* DB = new DiskBufferReader(this,theBufferNum*bufferSize,theBufferNum*bufferSize+bufferSize,fileHandle[theFileNum],theFilePos);
	bodys.insert(bodys.begin()+searchBegin,DB);
	DI = DB->localItem(itemNum);
	return DI;
}

unsigned DiskAsMemoryReader::createHeadBuffer(unsigned headBufferSize)
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
	specialHead = new DiskBufferReader(this,ULLONG_MAX,headBufferSize,0,0);
	return headBufferSize;
}

bool DiskAsMemoryReader::freeBuffer(DiskBufferReader *DB)
{
	vector<DiskBufferReader*>::iterator it;
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

DiskMultiItemReader* DiskAsMemoryReader::LocalMultiItem(uint64_t beginNum,unsigned itemNum) {
  if (beginNum+itemNum>*itemCounter || itemNum==0) {
    cerr<<"item number overflow "<<beginNum+itemNum<<" vs "<<*itemCounter<<endl;
    return NULL;
  }
  DiskMultiItemReader* multi_item = new DiskMultiItemReader(*itemCounter,itemSize);
  if(specialHead!=NULL && specialHead->end > beginNum) {
    unsigned cnum = itemNum;
    if (cnum > specialHead->end-beginNum) cnum = specialHead->end-beginNum;
    specialHead->PutMultiItems(multi_item,beginNum,cnum);
    itemNum -= cnum;
    beginNum += cnum;
  }
  while(itemNum > 0) {
    unsigned searchBegin=0,searchEnd=bodys.size(),searchMid;
    DiskBufferReader* cur_DB = NULL;
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
        fileHandle[theFileNum] = open(filename,O_RDONLY);
        if(fileHandle[theFileNum]==-1) {
          cerr<<"Could not read the old file! "<<theFileNum<<endl;
          return NULL;
        }
      }
      DiskBufferReader* DB = new DiskBufferReader(this,theBufferNum*bufferSize,theBufferNum*bufferSize+bufferSize,fileHandle[theFileNum],theFilePos);
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

DiskContentReader* DiskAsMemoryReader::LocalItemContent(uint64_t begin_num,unsigned item_num) {
  if (begin_num + item_num > *itemCounter || item_num == 0) return NULL;
  unsigned file_num_begin = begin_num * itemSize / MaxFileSize;
  unsigned file_pos_begin = begin_num * itemSize % MaxFileSize;
  unsigned file_num_end = (begin_num+item_num-1) * itemSize / MaxFileSize;
  unsigned page_size = sysconf(_SC_PAGESIZE);
  unsigned file_buffer_begin = file_pos_begin / page_size * page_size;
  if (file_num_begin == file_num_end) { // Normal content buffer;
    if(fileHandle[file_num_begin]==-1) {
      char filename[256];
      sprintf(filename,"%s/%d",foldPath,file_num_begin);
      fileHandle[file_num_begin] = open(filename,O_RDONLY);
      if(fileHandle[file_num_begin]==-1) {
        cerr<<"Could not read the old file! "<<file_num_begin<<endl;
        return NULL;
      }
    }
    unsigned offset = file_pos_begin - file_buffer_begin;
    unsigned buffer_length = offset + itemSize*item_num;
    char* buffer = (char*)mmap(0,buffer_length,PROT_READ,MAP_SHARED,fileHandle[file_num_begin],file_buffer_begin);
    if(buffer == MAP_FAILED)
    {
      cerr<<"Could not mmap the buffer!"<<file_buffer_begin<<" : "<<buffer_length<<endl;
      return NULL;
    }
    return new DiskContentReader(buffer,offset,itemSize,item_num);
  }
  // complicated content buffer across files.
  cout<<"Complicated buffer access!"<<endl;
  char* content = new char[itemSize*item_num];
  char* content_pointer = content;
  unsigned the_file_num = file_num_begin;
  unsigned the_pos = file_pos_begin;
  unsigned n = item_num;
  while (n>0) {
    unsigned buffer_pos = the_pos / page_size * page_size;
    unsigned offset = the_pos - buffer_pos;
    unsigned the_item_num = n;
    if (the_item_num > (MaxFileSize - the_pos)/itemSize) the_item_num = (MaxFileSize-the_pos)/itemSize;
    unsigned buffer_length = offset + itemSize * the_item_num;
    if(fileHandle[the_file_num]==-1) {
      char filename[256];
      sprintf(filename,"%s/%d",foldPath,the_file_num);
      fileHandle[the_file_num] = open(filename,O_RDONLY);
      if(fileHandle[the_file_num]==-1) {         
        cerr<<"Could not read the old file! "<<the_file_num<<endl;
        return NULL;
      }
    }
    char* buffer = (char*)mmap(0,buffer_length,PROT_READ,MAP_SHARED,fileHandle[the_file_num],buffer_pos);
    if(buffer == MAP_FAILED)
    {
      cerr<<"Could not mmap the buffer!"<<buffer_pos<<" : "<<buffer_length<<endl;
      delete[] content;
      return NULL;
    }
    memcpy(content_pointer,buffer+offset,itemSize*the_item_num);
    munmap(buffer,buffer_length);
    content_pointer += the_item_num * itemSize;
    n -= the_item_num;
    the_file_num++;
    the_pos = 0;
  }
  return new DiskContentReader(content,-1,itemSize,item_num);
}

char* DiskAsMemoryReader::CopyItems(uint64_t begin_num,unsigned item_num) {
  if (begin_num + item_num > *itemCounter || item_num == 0) return NULL;
  char* res = new char[item_num * itemSize];
  char* cur_pos = res;
  unsigned file_num_begin = begin_num * itemSize / MaxFileSize;
  unsigned file_pos_begin = begin_num * itemSize % MaxFileSize;
  unsigned page_size = sysconf(_SC_PAGESIZE);
  while (item_num > 0) {
    unsigned copy_num = item_num;
    if (item_num > (MaxFileSize - file_pos_begin) / itemSize) {
      copy_num = (MaxFileSize - file_pos_begin) / itemSize;
    }
    unsigned file_buffer_begin = file_pos_begin / page_size * page_size;
    unsigned offset = file_pos_begin - file_buffer_begin;
    unsigned buffer_length = offset + itemSize * copy_num;
    if(fileHandle[file_num_begin]==-1) {
      char filename[256];
      sprintf(filename,"%s/%d",foldPath,file_num_begin);
      fileHandle[file_num_begin] = open(filename,O_RDONLY);
      if(fileHandle[file_num_begin]==-1) {
        cerr<<"Could not read the old file! "<<file_num_begin<<endl;
        return NULL;
      }
    }
    char* buffer = (char*)mmap(0,buffer_length,PROT_READ,MAP_SHARED,fileHandle[file_num_begin],file_buffer_begin);
    if(buffer == MAP_FAILED) {
      cerr<<"Could not mmap the buffer!"<<file_buffer_begin<<" : "<<buffer_length<<endl;
      delete[] res;
      return NULL;
    }
    memcpy(cur_pos, buffer+offset, copy_num*itemSize);
    cur_pos += copy_num*itemSize;
    item_num -= copy_num;
    file_num_begin++;
    file_pos_begin=0;
    munmap(buffer,buffer_length);
  }
  return res;
}

  
    
