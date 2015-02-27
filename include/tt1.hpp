#include <iostream>

using namespace std;

template<class T>

bool ttCompare(char* buffer)
{
	T* content = (T *)buffer;
	return *content > *(content+1);
}
