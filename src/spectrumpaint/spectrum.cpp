
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

iqdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=35000; 
bool running=true;

void ProcessPicture(float Excursion)
{
		
	int EndOfPicture=0;
	int NbRead=0;
	int VIS=1;
	static unsigned char Line[320*3];
	std::complex<float> sample[320];
	int Row;
				
	//while(1)
	{
		while((EndOfPicture==0)&&(running==true))
		{
			NbRead=read(FilePicture,Line,320);
			if(NbRead!=320) EndOfPicture=1;
		
								
			for(Row=0;Row<320;Row++)
			{ 
				sample[Row]=std::complex<float>((Row/320.0-0.5)*Excursion,(Line[Row])/64);
				//sample[Row]=std::complex<float>((Row/320.0-0.5)*Excursion,(Row/320.0)*8);
				//fprintf(stderr,"%f ",sample[Row].imag());
				
			}
			//fprintf(stderr,"\n");
			fmmod->SetIQSamples(sample,320,1);
			
		}
		
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
	float Excursion=100000;
	if (argc > 2) 
	{
		char *sFilePicture=(char *)argv[1];
		FilePicture = open(argv[1], O_RDONLY);
		
		 frequency=atof(argv[2]);
	}
	if (argc > 3) 
	{
		Excursion=atof(argv[3]);
	}
	if(argc<2)
	{
		printf("usage : spectrum picture.rgb frequency(Hz) Excursion(Hz)\n");
		exit(0);
	}

	for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	fmmod=new iqdmasync(frequency,10000,14,FifoSize,MODE_FREQ_A);	
	ProcessPicture(Excursion);
    close(FilePicture);
	delete fmmod;
	return 0;
}
	



