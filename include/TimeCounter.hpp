#include <sys/time.h>
#include <iostream>

using namespace std;

class TimeCounter
{
public:
	TimeCounter();
	void start();
	double stop();
	void reset();
	inline double getTrackTime();
	inline double getTotalTime();
	inline double getAverageTime();
	inline int getTrackNum();
private:
	int num;
	bool runMark;
	double startTime;
	double totalTime;
	double trackTime;
};

TimeCounter::TimeCounter()
{
	reset();
}

void TimeCounter::start()
{
	if(runMark) {cerr<<"the time counter is already started"<<endl;return;}
	struct timeval tp;
	runMark=true;
	gettimeofday(&tp,NULL);
	startTime = tp.tv_sec*1e6+tp.tv_usec;
}

double TimeCounter::stop()
{
	if(!runMark) {cerr<<"the time counter is not started"<<endl;return 0;}
	struct timeval tp;
	gettimeofday(&tp,NULL);
	trackTime = tp.tv_sec*1e6+tp.tv_usec-startTime;
	if(trackTime<0) trackTime+=3600*24*1e6;
	totalTime+=trackTime;
	num++;
	runMark=false;
	return trackTime/1000;
}

void TimeCounter::reset()
{
	//if(runMark) {cerr<<"the time counter is already started"<<endl;return;}
	totalTime=0;
	num=0;
	startTime=0;
	trackTime=0;
	runMark=false;
}

inline double TimeCounter::getTrackTime() {return trackTime/1000;}
inline double TimeCounter::getTotalTime() {return totalTime/1000;}
inline double TimeCounter::getAverageTime() {return (num!=0)?(totalTime/num)/1000:0;}
inline int TimeCounter::getTrackNum() {return num;}
