struct MinHeapItem
{
	unsigned docID;
	float score;
};

class MinHeap
{
public:
	unsigned size;
	float smallest;
	unsigned n;

	MinHeap(unsigned s);
	unsigned push(unsigned docID,float score);
	bool remove(unsigned docID);
	bool increase(unsigned docID,float score);
	bool increaseAdd(unsigned docID,float score);
	unsigned pop(float &score);
	MinHeap* copy();
	~MinHeap();
private:
	struct MinHeapItem *items;
	
	void inline heapify(unsigned t);
};

MinHeap::MinHeap(unsigned s)
{
	size=s;
	items=new MinHeapItem[size+1];
	n=0;
	smallest=-2000000000;
}

void inline MinHeap::heapify(unsigned t)
{
	unsigned left= t*2;
	unsigned right= t*2+1;
	unsigned minT=t;
	if(left<=n && items[left].score < items[minT].score) minT=left;
	if(right<=n && items[right].score < items[minT].score) minT=right;
	if(minT!=t)
	{
		unsigned tempD=items[t].docID;
		float tempS=items[t].score;
		items[t].docID=items[minT].docID;
		items[t].score=items[minT].score;
		items[minT].docID=tempD;
		items[minT].score=tempS;
		heapify(minT);
	}
}

unsigned MinHeap::push(unsigned docID,float score)
{
	unsigned minK=0xFFFFFFFF;
	if(n>=size) //heap full
	{
		minK = items[1].docID;
		items[1].docID=docID;
		items[1].score=score;
		heapify(1);
	}
	else
	{	
		n++;
		int p=n;
		while(p > 1 && items[p/2].score > score)
		{
			items[p].docID=items[p/2].docID;
			items[p].score=items[p/2].score;
			p/=2;
		}
		items[p].docID = docID;
		items[p].score=score;
	}
	if(n>=size) {smallest=items[1].score;}
	return minK;
}

bool MinHeap::increase(unsigned docID,float score)
{
	unsigned i;
	for(i=1;i<=n;i++)
	{
		if(items[i].docID==docID)
		{
			items[i].score=score;
			heapify(i);
			if(n>=size) smallest=items[1].score;
			return true;
		}
	}
	return false;
}

bool MinHeap::increaseAdd(unsigned docID,float score)
{
	unsigned i;
	for(i=1;i<=n;i++)
	{
                if(items[i].docID==docID)
                {
                        items[i].score += score;
                        heapify(i);
                        if(n>=size) smallest=items[1].score;
                        return true;
                }
        }
        return false;
}

MinHeap* MinHeap::copy()
{
	MinHeap* MH=new MinHeap(size);
	MH->n=n;
	MH->smallest=smallest;
	int i;
	for(i=1;i<=n;i++)
	{
		MH->items[i].docID=items[i].docID;
		MH->items[i].score=items[i].score;
	}
	return MH;
}

bool MinHeap::remove(unsigned docID)
{
	unsigned i;
	for(i=1;i<=n;i++)
	{
		if(items[i].docID==docID)
		{
			items[i].docID=items[n].docID;
			items[i].score=items[n].score;
			n--;
			heapify(i);
			if(i==1) smallest=items[1].score;
			return true;
		}
	}
	return false;
}

unsigned MinHeap::pop(float &score)
{
	if(n<1) return -1;
	unsigned docID=items[1].docID;
	score=items[1].score;
	if(n>1)
	{
		items[1].docID=items[n].docID;
		items[1].score=items[n].score;
		n--;
		heapify(1);
	}
	else n--;
	return docID;
}

MinHeap::~MinHeap()
{
	delete[] items;
}
