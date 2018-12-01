#include <unistd.h>
#include "librpitx/src/librpitx.h"
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>

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
-t            IQ type (i16 default) {i16,u8,float,double}\n\
-?            help (this help).\n\
\n",\
PROGRAM_VERSION);

} /* end function print_usage */

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating %x\n",num);
   
}

#define MAX_SAMPLERATE 200000

int main(int argc, char* argv[])
{
	int a;
	int anyargs = 1;
	float SetFrequency=434e6;
	float SampleRate=48000;
	bool loop_mode_flag=false;
	char* FileName=NULL;
	int Harmonic=1;
	enum {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double};
	int InputType=typeiq_i16;
	int Decimation=1;
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
			if(SampleRate>MAX_SAMPLERATE) 
			{
				
				for(int i=2;i<12;i++) //Max 10 times samplerate
				{
					if(SampleRate/i<MAX_SAMPLERATE) 
					{
						SampleRate=SampleRate/i;
						Decimation=i;
						break;
					}	
				}
				if(Decimation==1)
				{	
					 fprintf(stderr,"SampleRate too high : >%d sample/s",10*MAX_SAMPLERATE);
					 exit(1);
				}	 
				else
				{	
					fprintf(stderr,"Warning samplerate too high, decimation by %d will be performed",Decimation);	 
				}		
			};	
			break;
		case 'h': // help
			Harmonic=atoi(optarg);
			break;
		case 'l': // loop mode
			loop_mode_flag = true;
			break;
		case 't': // inout type
			if(strcmp(optarg,"i16")==0) InputType=typeiq_i16;
			if(strcmp(optarg,"u8")==0) InputType=typeiq_u8;
			if(strcmp(optarg,"float")==0) InputType=typeiq_float;
			if(strcmp(optarg,"double")==0) InputType=typeiq_double;
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

	if(FileName==NULL) {fprintf(stderr,"Need an input\n");exit(1);}
	
	 for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	FILE *iqfile=NULL;
	if(strcmp(FileName,"-")==0)
		iqfile=fopen("/dev/stdin","rb");
	else	
		iqfile=fopen(FileName	,"rb");
	if (iqfile==NULL) 
	{	
		printf("input file issue\n");
		exit(0);
	}

	#define IQBURST 4000
	
	int SR=48000;
	int FifoSize=IQBURST*4;
	iqdmasync iqtest(SetFrequency,SampleRate,14,FifoSize,MODE_IQ);
	iqtest.SetPLLMasterLoop(3,4,0);
	//iqtest.print_clock_tree();
	//iqtest.SetPLLMasterLoop(5,6,0);
	
	std::complex<float> CIQBuffer[IQBURST];	
	while(running)
	{
		
			int CplxSampleNumber=0;
			switch(InputType)
			{
				case typeiq_i16:
				{
					static short IQBuffer[IQBURST*2];
					int nbread=fread(IQBuffer,sizeof(short),IQBURST*2,iqfile);
					//if(nbread==0) continue;
					if(nbread>0)
					{
						for(int i=0;i<nbread/2;i++)
						{
							if(i%Decimation==0)
							{		
								CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2]/32768.0,IQBuffer[i*2+1]/32768.0); 
							}
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
					
					if(nbread>0)
					{
						for(int i=0;i<nbread/2;i++)
						{
							if(i%Decimation==0)
							{	
								CIQBuffer[CplxSampleNumber++]=std::complex<float>((IQBuffer[i*2]-127.5)/128.0,(IQBuffer[i*2+1]-127.5)/128.0);
										
							}		 
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
				case typeiq_float:
				{
					static float IQBuffer[IQBURST*2];
					int nbread=fread(IQBuffer,sizeof(float),IQBURST*2,iqfile);
					//if(nbread==0) continue;
					if(nbread>0)
					{
						for(int i=0;i<nbread/2;i++)
						{
							if(i%Decimation==0)
							{	
								CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2],IQBuffer[i*2+1]);
										
							}		 
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
				case typeiq_double:
				{
					static double IQBuffer[IQBURST*2];
					int nbread=fread(IQBuffer,sizeof(double),IQBURST*2,iqfile);
					//if(nbread==0) continue;
					if(nbread>0)
					{
						for(int i=0;i<nbread/2;i++)
						{
							if(i%Decimation==0)
							{	
								CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2],IQBuffer[i*2+1]);
										
							}		 
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
		iqtest.SetIQSamples(CIQBuffer,CplxSampleNumber,Harmonic);
	}

	iqtest.stop();
	
}	

