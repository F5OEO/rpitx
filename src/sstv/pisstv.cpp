
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
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

int FilePicture;
int FileFreqTiming;

ngfmdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=10000; //10ms
bool running=true;

void playtone(double Frequency,uint32_t Timing)//Timing in 0.1us
{
		uint32_t SumTiming=0;
		SumTiming+=Timing%100;
 		if(SumTiming>=100) 
		{
			Timing+=100;
			SumTiming=SumTiming-100;
		}
		int NbSamples=(Timing/100);

		while(NbSamples>0)
		{
			usleep(10);
			int Available=fmmod->GetBufferAvailable();
			if(Available>FifoSize/2)
			{	
				int Index=fmmod->GetUserMemIndex();
				if(Available>NbSamples) Available=NbSamples;
				for(int j=0;j<Available;j++)
				{
					fmmod->SetFrequencySample(Index+j,Frequency);
					NbSamples--;
			
				}
			}
		}	
}

void addvisheader()
{
	printf( "Adding VIS header to audio data.\n" ) ;
	
	// bit of silence
	playtone(    0 , 5000000) ;   
	
	// attention tones
	playtone( 1900 , 100000 ) ; // you forgot this one
	playtone( 1500 , 1000000) ;
	playtone( 1900 , 1000000) ;
	playtone( 1500 , 1000000) ;
	playtone( 2300 , 1000000) ;
	playtone( 1500 , 1000000) ;
	playtone( 2300 , 1000000) ;
	playtone( 1500 , 1000000) ;
	               
	// VIS lead, break, mid, start
	playtone( 1900 , 3000000) ;
	playtone( 1200 ,  100000) ;
	//playtone( 1500 , 300000 ) ;
	playtone( 1900 , 3000000) ;
	playtone( 1200 ,  300000) ;
	
	// VIS data bits (Martin 1)
	playtone( 1300 ,  300000) ;
	playtone( 1300 ,  300000) ;
	playtone( 1100 ,  300000) ;
	playtone( 1100 ,  300000) ;
	playtone( 1300 ,  300000) ;
	playtone( 1100 ,  300000) ;
	playtone( 1300 ,  300000 ) ;
	playtone( 1100 ,  300000 ) ; 
	
	// VIS stop
	playtone( 1200 ,  300000 ) ; 
	
	printf( "Done adding VIS header to audio data.\n" ) ;
        
} // end addvisheader   

void addvistrailer ()
{
	printf( "Adding VIS trailer to audio data.\n" ) ;
	
	playtone( 2300 , 3000000 ) ;
	playtone( 1200 ,  100000 ) ;
	playtone( 2300 , 1000000 ) ;
	playtone( 1200 ,  300000 ) ;
	
	// bit of silence
	playtone(    0 , 5000000 ) ;
	
	printf( "Done adding VIS trailer to audio data.\n" ) ;    
}

void ProcessMartin1()
{
	static uint32_t FrequencyMartin1[3]={1200,1500,1500};
	static uint32_t TimingMartin1[3]={48620,5720,4576};
	
	int EndOfPicture=0;
	int NbRead=0;
	int VIS=1;
	static unsigned char Line[320*3];
	
	int Row;
				
	addvisheader();
	addvistrailer();
	while((EndOfPicture==0)&&(running==true))
	{
		NbRead=read(FilePicture,Line,320*3);
		if(NbRead!=320*3) EndOfPicture=1;
		//MARTIN 1 Implementation
		
		//Horizontal SYNC
		playtone((double)FrequencyMartin1[0],TimingMartin1[0]);
		
		//Separator Tone
		playtone((double)FrequencyMartin1[1],TimingMartin1[1]);
				
		for(Row=0;Row<320;Row++)
		{ 
			playtone((double)FrequencyMartin1[1]+Line[Row*3+1]*800/256,TimingMartin1[2]);
		}
		playtone((double)FrequencyMartin1[1],TimingMartin1[1]);
				
		//Blue
		for(Row=0;Row<320;Row++)
		{ 
			playtone((double)FrequencyMartin1[1]+Line[Row*3+2]*800/256,TimingMartin1[2]);
		}
		playtone((double)FrequencyMartin1[1],TimingMartin1[1]);
			
		//Red
		for(Row=0;Row<320;Row++)
		{ 
			playtone((double)FrequencyMartin1[1]+Line[Row*3]*800/256,TimingMartin1[2]);
		}
		playtone((double)FrequencyMartin1[1],TimingMartin1[1]);
	}
}


static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating %x\n",num);
   
}

int main(int argc, char **argv)
{
	float frequency=144.5e6;
	if (argc > 2) 
	{
		char *sFilePicture=(char *)argv[1];
		FilePicture = open(argv[1], O_RDONLY);
		
		 frequency=atof(argv[2]);
		
	}
	else
	{
		printf("usage : pisstv picture.rgb frequency(Hz)\n");
		exit(0);
	}
	
	for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	fmmod=new ngfmdmasync(frequency,100000,14,FifoSize);	
	ProcessMartin1();
    close(FilePicture);
	delete fmmod;
	return 0;
}
	



