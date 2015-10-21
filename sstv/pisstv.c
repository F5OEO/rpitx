
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

int FilePicture;
int FileFreqTiming;
static double GlobalTuningFrequency=00000.0;

void playtone(double Frequency,uint32_t Timing)
{
	
	typedef struct {
		double Frequency;
		uint32_t WaitForThisSample;
	} samplerf_t;
	samplerf_t RfSample;
	

	RfSample.Frequency=GlobalTuningFrequency+Frequency;
	RfSample.WaitForThisSample=Timing*100L; //en 100 de nanosecond
	//printf("Freq =%f Timing=%d\n",RfSample.Frequency,RfSample.WaitForThisSample);
	write(FileFreqTiming,&RfSample,sizeof(samplerf_t));

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
//    playtone( 1500 , 300000 ) ;
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
	static uint32_t TimingMartin1[3]={48720,5720,4576};
	
	
	
	int EndOfPicture=0;
	int NbRead=0;
	int VIS=1;
	static unsigned char Line[320*3];
	
	
				int Row;
				
				
				addvisheader();
				addvistrailer();
				while(EndOfPicture==0)
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


int
main(int argc, char **argv)
{

if (argc > 2) {
		
		char *sFilePicture=(char *)argv[1];
	       	FilePicture = open(argv[1], 'r');

		char *sFileFreqTiming=(char *)argv[2];
	       	FileFreqTiming = open(argv[2],O_WRONLY|O_CREAT);
		}
		else
		{
			printf("usage : pisstv picture.rgb outputfreq.bin\n");
			exit(0);
		}
		
		ProcessMartin1();
		close(FilePicture);
		close(FileFreqTiming);
		return 0;
}
	



