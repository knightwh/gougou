#include <iostream>
#include "porter.hpp"

using namespace std;

StemTerm::StemTerm(char* t) {strcpy(term,t);stem();}
StemTerm::StemTerm(string s) {strcpy(term,s.c_str());stem();}

char* StemTerm::stem(char* t)
{
  if(t=='\0') {
    *term = '\0';
    return term;
  }
	strcpy(term,t);stem();
	return term;
}

string StemTerm::stem(const string& s)
{
  if (s.size()==0) return s;
	strcpy(term,s.c_str());
  stem();
  string strs(term);
	//ostringstream strs;
    //    strs << term;
  return strs;
}
	

bool StemTerm::ends(char* s)
{
	int length = s[0];
   	if (length > k+1) return false;
	if (memcmp(term+k-length+1,s+1,length)!=0) return false;
	j = k-length;
	return true;

}

void StemTerm::setto(char *s)
{
	int length = s[0];
	memmove(term+j+1,s+1,length);
	k = j+length;
}

void StemTerm::rsetto(char *s) {if(mConsonant() > 0) setto(s);}

bool StemTerm::cons(int i)
{
	switch (term[i])
	{
		case 'a': case 'e': case 'i': case 'o': case 'u': return false;
		case 'y': return (i==0) ? true : !cons(i-1);
		default: return true;
	}
}

int StemTerm::mConsonant()
{
	int n = 0;
	int i = 0;
	while(true)
	{
		if(i>j) return 0;
		if(!cons(i)) break;
		i++;
	}
	i++;
	while(true)
	{
		while(true)
		{
			if(i>j) return n;
			if (cons(i)) break;
			i++;
		}
		i++;
		n++;
		while(true)
		{
			if(i>j) return n;
			if(!cons(i)) break;
			i++;
		}
		i++;
	}
}

bool StemTerm::vowelinStem()
{
	for(int i=0;i<=j;i++) if(!cons(i)) return true;
	return false;
}

bool StemTerm::cvc(int i)
{
	if(i<2 || !cons(i) || cons(i-1) || !cons(i-2)) return false;
	int ch = term[i];
	if(ch == 'w' || ch == 'x' || ch=='y') return false;
	return true;
}

void StemTerm::stem()
{
	unsigned int i;
	for(i=0;i<strlen(term);i++) term[i]=tolower(term[i]);
	k = strlen(term)-1;
	j = k;
	if(k<=1) return;
	
	//step lab
	if(term[k] == 's')
	{
		if(ends((char*)"\04""sses")) k-= 2;
		else if(ends((char*)"\03" "ies")) setto((char*)"\01" "i");
		else if(term[k-1] != 's') k--;
	}
	if(ends((char*)"\03" "eed")) { if(mConsonant() >0) k--;}
	else if((ends((char*)"\02" "ed") || ends((char*)"\03" "ing")) && vowelinStem())
	{
		k=j;
		if (ends((char*)"\02" "at")) setto((char*)"\03" "ate");
		else if(ends((char*)"\02" "bl")) setto((char*)"\03" "ble");
		else if(ends((char*)"\02" "iz")) setto((char*)"\03" "ize");
		else if(k>1 && term[k]==term[k-1] && cons(k))
		{
			k--;
			int ch = term[k];
			if( ch =='l' || ch=='s' || ch == 'z') k++;
		}
		else if(mConsonant() == 1 && cvc(k)) setto((char*)"\01" "e");
	}
	//steplc
	if(ends((char*)"\01" "y") && vowelinStem()) term[k] = 'i';

	//step 2
	switch (term[k-1])
	{
		case 'a': if(ends((char*)"\07" "ational")) {rsetto((char*)"\03" "ate");break;}
			  if(ends((char*)"\06" "tional")) {rsetto((char*)"\04" "tion");break;}
			  break;
		case 'c': if(ends((char*)"\04" "enci")) {rsetto((char*)"\04" "ence");break;}
			  if(ends((char*)"\04" "anci")) {rsetto((char*)"\04" "ance");break;}
			  break;
		case 'e': if (ends((char*)"\04" "izer")) { rsetto((char*)"\03" "ize"); break; }
			  break;
		case 'l': if (ends((char*)"\03" "bli")) { rsetto((char*)"\03" "ble"); break; }
			  if (ends((char*)"\04" "alli")) { rsetto((char*)"\02" "al"); break; }
			  if (ends((char*)"\05" "entli")) { rsetto((char*)"\03" "ent"); break; }
			  if (ends((char*)"\03" "eli")) { rsetto((char*)"\01" "e"); break; }
			  if (ends((char*)"\05" "ousli")) { rsetto((char*)"\03" "ous"); break; }
			  break;
		case 'o': if (ends((char*)"\07" "ization")) { rsetto((char*)"\03" "ize"); break; }
			  if (ends((char*)"\05" "ation")) { rsetto((char*)"\03" "ate"); break; }
			  if (ends((char*)"\04" "ator")) { rsetto((char*)"\03" "ate"); break; }
			  break;
		case 's': if (ends((char*)"\05" "alism")) { rsetto((char*)"\02" "al"); break; }
			  if (ends((char*)"\07" "iveness")) { rsetto((char*)"\03" "ive"); break; }
			  if (ends((char*)"\07" "fulness")) { rsetto((char*)"\03" "ful"); break; }
			  if (ends((char*)"\07" "ousness")) { rsetto((char*)"\03" "ous"); break; }
			  break;
		case 't': if (ends((char*)"\05" "aliti")) { rsetto((char*)"\02" "al"); break; }
			  if (ends((char*)"\05" "iviti")) { rsetto((char*)"\03" "ive"); break; }
			  if (ends((char*)"\06" "biliti")) { rsetto((char*)"\03" "ble"); break; }
			  break;
		case 'g': if (ends((char*)"\04" "logi")) { rsetto((char*)"\03" "log"); break; }
	}
	
	//step 3
	switch (term[k])
	{
		case 'e': if (ends((char*)"\05" "icate")) { rsetto((char*)"\02" "ic"); break; }
			  if (ends((char*)"\05" "ative")) { rsetto((char*)"\00" ""); break; }
			  if (ends((char*)"\05" "alize")) { rsetto((char*)"\02" "al"); break; }
			  break;
		case 'i': if (ends((char*)"\05" "iciti")) { rsetto((char*)"\02" "ic"); break; }
			  break;
		case 'l': if (ends((char*)"\04" "ical")) { rsetto((char*)"\02" "ic"); break; }
			  if (ends((char*)"\03" "ful")) { rsetto((char*)"\00" ""); break; }
			  break;
		case 's': if (ends((char*)"\04" "ness")) { rsetto((char*)"\00" ""); break; }
			  break;
	}

	j=k;
	//step 4
	switch (term[k-1])
	{
		case 'a': ends((char*)"\02" "al");break;
		case 'c': if(ends((char*)"\04" "ance")) break;
			  ends((char*)"\04" "ence");break;
		case 'e': ends((char*)"\02" "er");break;
		case 'i': ends((char*)"\02" "ic");break;
		case 'l': if(ends((char*)"\04" "able")) break;
			  ends((char*)"\04" "ible");break;
		case 'n': if(ends((char*)"\03" "ant")) break;
			  if(ends((char*)"\05" "ement")) break;
			  if(ends((char*)"\04" "ment")) break;
			  ends((char*)"\03" "ent");break;
		case 'o': if(ends((char*)"\03" "ion"))
			  {
				if(j<0 || (term[j] != 's' && term[j] != 't')) j=k;break;
			  }
			  ends((char*)"\02" "ou");break;
		case 's': ends((char*)"\03" "ism");break;
		case 't': if(ends((char*)"\03" "ate")) break;
			  ends((char*)"\03" "iti");break;
		case 'u': ends((char*)"\03" "ous");break;
		case 'v': ends((char*)"\03" "ive");break;
		case 'z': ends((char*)"\03" "ize");break;
	}
	if(mConsonant() > 1) k=j;

	//step 5
	j=k;
	if(term[k] == 'e')
	{
		int a = mConsonant();
		if(a>1 || (a==1 && !cvc(k-1))) k--;
	}
	if (term[k] == 'l' && mConsonant()>1 && (k>1 && term[k]==term[k-1] && cons(k))) k--;
	term[k+1]='\0';
}
	
string StemTerm::output()
{
	ostringstream strs;
	strs << term;
	return strs.str();
}

