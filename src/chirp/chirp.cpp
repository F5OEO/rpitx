#include <unistd.h>
#include "../librpitx/src/librpitx.h"
#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>

bool running=true;


void SimpleTestDMA(uint64_t Freq)
{
	

	int SR=1000;
	int FifoSize=4096;
	ngfmdmasync ngfmtest(Freq,SR,14,FifoSize);
	for(int i=0;running;)
	{
		//usleep(10);
		usleep(FifoSize*1000000.0*3.0/(4.0*SR));
		int Available=ngfmtest.GetBufferAvailable();
		if(Available>FifoSize/2)
		{	
			int Index=ngfmtest.GetUserMemIndex();
			//printf("GetIndex=%d\n",Index);
			for(int j=0;j<Available;j++)
			{
				//ngfmtest.SetFrequencySample(Index,((i%10000)>5000)?1000:0);
				ngfmtest.SetFrequencySample(Index+j,(i%SR)/10.0);
				i++;
			
			}
		}
		
		
	}
	fprintf(stderr,"End\n");
	
	ngfmtest.stop();
	
}



static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
}


int main(int argc, char* argv[])
{
	
	if(argc<3) {printf("Usage : pichirp Frequency(Hz) Bandwidth(Hz) Time(Seconds)\n");exit(0);}
	
	float Frequency=atof(argv[1]);
	float Bandwidth=atof(argv[2]);
	float Time=atof(argv[3]);

	 for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }
	
	int SR=100000;
	 
	int FifoSize=4096;
	ngfmdmasync ngfmtest(Frequency,SR,14,FifoSize);
	float Step=Bandwidth/Time;	//Deviation Hz by second 
	float StepWithSR=Step/(float)SR;
	int NbStepWithSR=Time*SR;
	float FrequencyDeviation=0;
	int count=0;
	for(int i=0;running;)
	{
		
		usleep(FifoSize*1000000.0*3.0/(4.0*SR));
		int Available=ngfmtest.GetBufferAvailable();
		if(Available>FifoSize/2)
		{	
			int Index=ngfmtest.GetUserMemIndex();
			
			for(int j=0;j<Available;j++)
			{
				
				ngfmtest.SetFrequencySample(Index+j,StepWithSR*count);
				count++;
				if(count>NbStepWithSR) count=0;
			
			}
			
		}
		fprintf(stderr,"Freq=%f\n",Frequency+StepWithSR*count);
		
	}
	fprintf(stderr,"End\n");
	
	ngfmtest.stop();

	
	
	
}	
