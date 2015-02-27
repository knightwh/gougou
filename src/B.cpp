#include <iostream>
#include "B.hpp"

using namespace std;

B::B(unsigned d)
{
	v=d;
}

void B::show()
{
	A* a = new A(v);
	cout<<a->v<<endl;
	delete a;
}
