
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

int FilePicture;
int FileFreqTiming;

iqdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=35000; 


void ProcessPicture(float Excursion)
{
		
	int EndOfPicture=0;
	int NbRead=0;
	int VIS=1;
	static unsigned char Line[320*3];
	std::complex<float> sample[320];
	int Row;
				
	
	while(EndOfPicture==0)
	{
		NbRead=read(FilePicture,Line,320);
		if(NbRead!=320) EndOfPicture=1;
	
							
		for(Row=0;Row<320;Row++)
		{ 
			sample[Row]=std::complex<float>((Row/320.0-0.5)*Excursion,(Line[Row])/4);
			
			
		}
		fmmod->SetIQSamples(sample,320,1);
		
	}
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
	
	fmmod=new iqdmasync(frequency,10000,14,FifoSize,MODE_FREQ_A);	
	ProcessPicture(Excursion);
    close(FilePicture);
	delete fmmod;
	return 0;
}
	



