
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../librpitx/src/librpitx.h"

int FileVCO;

ngfmdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=10000; //10ms

void playtone(float Frequency)
{
		float VCOFreq[100];
		for(int i=0;i<100;i++) VCOFreq[i]=Frequency;
		fmmod->SetFrequencySamples(VCOFreq,100);
		
}


int main(int argc, char **argv)
{
	float frequency=144.5e6;
	int SampleRate=400;
	if (argc >2 ) 
	{
		char *sFileVCO=(char *)argv[1];
		FileVCO = open(sFileVCO, O_RDONLY);
		
		 frequency=atof(argv[2]);
		
	}
	if (argc >3 ) 
	{
		 SampleRate=(int)atof(argv[3]);
		
	}
	if(argc<=2)
	{
		printf("usage : freedv vco.rf frequency(Hz) samplerate(Hz)\n");
		exit(0);
	}
	
	fmmod=new ngfmdmasync(frequency,100*SampleRate,14,FifoSize); //400 bits*100 for 800XA	
	int ByteRead=1;
	short VCOFreq;
	while(ByteRead>0)
	{
		ByteRead=read(FileVCO,&VCOFreq,sizeof(short));
		if(ByteRead==sizeof(short))
		{
			
			playtone(VCOFreq);
		}
		/*else
		{
			lseek(FileVCO,0,SEEK_SET);
			ByteRead=1;	
		}*/	
	}
	printf("End of Tx\n");
    close(FileVCO);
	delete fmmod;
	return 0;
}
	



