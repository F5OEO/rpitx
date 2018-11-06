
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
bool running=true;

ngfmdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=10000; //10ms

void playtone(float Frequency)
{
		float VCOFreq[100];
		for(int i=0;i<100;i++) VCOFreq[i]=Frequency;
		fmmod->SetFrequencySamples(VCOFreq,100);
		
}

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
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

	for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	fmmod=new ngfmdmasync(frequency,100*SampleRate,14,FifoSize); //400 bits*100 for 800XA	
	padgpio pad;
	pad.setlevel(7);// Set max power
	fmmod->enableclk(20);//CLK1 duplicate on GPIO20 for more power ?
	
	short VCOFreq;
	while(running)
	{
		int ByteRead=read(FileVCO,&VCOFreq,sizeof(short));
		if(ByteRead==sizeof(short))
		{
			
			playtone(VCOFreq);
		}
		else
			running=false;
		/*else
		{
			lseek(FileVCO,0,SEEK_SET);
			ByteRead=1;	
		}*/	
	}
	fmmod->disableclk(20);
	printf("End of Tx\n");
    close(FileVCO);
	delete fmmod;
	return 0;
}
	



