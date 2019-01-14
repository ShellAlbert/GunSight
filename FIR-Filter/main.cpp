#include <iostream>
#include <fstream>
#include "DigitalFilters.h"
using namespace std;

int main(int argc, char** argv){
	constexpr float dtUsed = 0.01;
	// Construct various filter with cutoff frequency of 0.5 Hz.
	LowPassFilter lpf1(dtUsed, M_PI);
        FILE *fp=fopen("a.pcm","r");
 	if(NULL==fp)
	{
		cout<<"failed to open a.pcm"<<endl;
		return -1;
	}
	while(!feof(fp))
	{
		double input;
		char *pInput=(char*)&input;
		if(fread((void*)pInput,sizeof(input),1,fp)!=1)
		{
			cout<<"failed to fread()."<<endl;
			break;
		}
		cout<<input<<" -> "<<lpf1.update(input)<<endl;
	}
	fclose(fp);
	cout<<"done.";
#if 0
	LowPassFilter2 lpf2(dtUsed, 1/(M_PI));
	LowPassFilter3 lpf3(dtUsed, M_PI);
	HighPassFilter hpf1(dtUsed, M_PI);
	HighPassFilter3 hpf3(dtUsed, M_PI);
	double input;

	for (int i = 0; i < 1000; ++i)
	{
		// Create 1 unit step after 1 second.
		if(i < 100)
			input = 0;
		else
			input = 1;
		cout.width(8);
		cout << i * dtUsed
		<< ", " << input
		<< ", " << lpf1.update(input)
		<< ", " << lpf2.update(input)
		<< ", " << lpf3.update(input)
		<< ", " << hpf1.update(input)
		<< ", " << hpf3.update(input)
		<< endl;
	}
#endif
	return 0;
}
