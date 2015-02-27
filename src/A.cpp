#include <iostream>
#include "A.hpp"

using namespace std;

A::A(unsigned d)
{
	v=d;
}

void A::show()
{
	B* b = new B(v);
	cout<<b->v<<endl;
	delete b;
}
