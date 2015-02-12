#ifndef GOUGOU_PORTER_
#define GOUGOU_PORTER_
#include <string.h>
#include <string>
#include <sstream>

using namespace std;

class StemTerm
{
public:
	char term[2048];
	StemTerm() {term[0]='\0';}
	StemTerm(char* t);
	StemTerm(string s);
	string output();
	char* stem(char* t);
	string stem(const string& s);
private:
	int k;//the last position of the string
	int j;//the last position for judgement
	void stem();//main function to stem the term
	bool ends(char*);//judge whether the term is end with certain string
	void setto(char *);//set the things after judgement postions as string
	void rsetto(char *);//setto if it contains consonant
	int mConsonant();//count the number of consonant sequence
	bool cons(int);//judge whether the character is a consonant
	bool vowelinStem();//judge whether there is vowel in the term
	bool cvc(int);
};

#endif
