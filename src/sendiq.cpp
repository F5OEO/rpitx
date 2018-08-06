#include <unistd.h>
#include "librpitx/src/librpitx.h"
#include "stdio.h"
#include <cstring>
#include <signal.h>

bool running=true;

#define PROGRAM_VERSION "0.1"

void SimpleTestFileIQ(uint64_t Freq)
{
	
}

void print_usage(void)
{

fprintf(stderr,\
"\nsendiq -%s\n\
Usage:\nsendiq [-i File Input][-s Samplerate][-l] [-f Frequency] [-h Harmonic number] \n\
-i            path to File Input \n\
-s            SampleRate 10000-250000 \n\
-f float      central frequency Hz(50 kHz to 1500 MHz),\n\
-l            loop mode for file input\n\
-h            Use harmonic number n\n\
-?            help (this help).\n\
\n",\
PROGRAM_VERSION);

} /* end function print_usage */

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
}
  
int main(int argc, char* argv[])
{
	int a;
	int anyargs = 1;
	float SetFrequency=434e6;
	float SampleRate=48000;
	bool loop_mode_flag=false;
	char* FileName=NULL;
	int Harmonic=1;
	enum {typeiq_i16,typeiq_u8};
	int InputType=typeiq_i16;
	while(1)
	{
		a = getopt(argc, argv, "i:f:s:h:lt:");
	
		if(a == -1) 
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(a)
		{
		case 'i': // File name
			FileName = optarg;
			break;
		case 'f': // Frequency
			SetFrequency = atof(optarg);
			break;
		case 's': // SampleRate (Only needeed in IQ mode)
			SampleRate = atoi(optarg);
			break;
		case 'h': // help
			Harmonic=atoi(optarg);
			break;
		case 'l': // loop mode
			loop_mode_flag = true;
			break;
		case 't': // inout type
			if(strcmp(optarg,"u8")==0) InputType=typeiq_u8;
			break;
		case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 			{
 				fprintf(stderr, "sendiq: unknown option `-%c'.\n", optopt);
 			}
			else
			{
				fprintf(stderr, "sendiq: unknown option character `\\x%x'.\n", optopt);
			}
			print_usage();

			exit(1);
			break;			
		default:
			print_usage();
			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */

	
	
	 for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	FILE *iqfile=NULL;
	iqfile=fopen(FileName	,"rb");
	if (iqfile==NULL) 
	{	
		printf("input file issue\n");
		exit(0);
	}

	#define IQBURST 1000
	
	int SR=48000;
	int FifoSize=IQBURST*4;
	iqdmasync iqtest(SetFrequency,SampleRate,14,FifoSize);
	iqtest.SetPLLMasterLoop(3,4,0);
	iqtest.print_clock_tree();
	//iqtest.SetPLLMasterLoop(5,6,0);
	
	std::complex<float> CIQBuffer[IQBURST];	
	while(running)
	{
		
		
			switch(InputType)
			{
				case typeiq_i16:
				{
					static short IQBuffer[IQBURST*2];
					int nbread=fread(IQBuffer,sizeof(short),IQBURST*2,iqfile);
					if(nbread==IQBURST*2)
					{
						for(int i=0;i<nbread/2;i++)
						{
					
							CIQBuffer[i]=std::complex<float>(IQBuffer[i*2]*10/32768.0,IQBuffer[i*2+1]*10/32768.0); 
				
						}
					}
					else 
					{
						printf("End of file\n");
						if(loop_mode_flag)
						fseek ( iqfile , 0 , SEEK_SET );
						else
							running=false;
					}
				}
				break;
				case typeiq_u8:
				{
					static unsigned char IQBuffer[IQBURST*2];
					int nbread=fread(IQBuffer,sizeof(unsigned char),IQBURST*2,iqfile);
					if(nbread==IQBURST*2)
					{
						for(int i=0;i<nbread/2;i++)
						{
					
							CIQBuffer[i]=std::complex<float>((IQBuffer[i*2]-127.5)/128.0,(IQBuffer[i*2+1]-127.5)/128.0); 
							//printf("%f %f\n",(IQBuffer[i*2]-127.5)/128.0,(IQBuffer[i*2+1]-127.5)/128.0);
						}
					}
					else 
					{
						printf("End of file\n");
						if(loop_mode_flag)
						fseek ( iqfile , 0 , SEEK_SET );
						else
							running=false;
					}
				}
				break;	
			
		}
		iqtest.SetIQSamples(CIQBuffer,IQBURST,Harmonic);
	}

	iqtest.stop();
	
}	

