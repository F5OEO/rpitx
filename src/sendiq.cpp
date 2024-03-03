#include <unistd.h>
#include <librpitx/librpitx.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>		//Used for shared memory
// =============================================================================================
// ----- SHARED MEMORY STRUCTURE -----
// All commands sent by a partner program towards sendiq
// Operation tips
//      1-Partner program creates a shared memory block with a given arbitrary token id
//        (any integer non-zero integer number).
//      2-sendiq is started with the -m {token} argument, without it no shared memory commands
//        are honored.
//      3-Partner program willing to alter the operation of sendiq place the
//        appropriate commnand plus optional associated data and/or common_data
//        if needed (see table below).
//      4-Partner program set updated=true.
//      5-sendiq checks periodically for condition updated==true.
//      6-if detected updated==true at step -5- the command is evaluated and executed.
//      7-sendiq set updated=false.
//      8-Partner program must refrain to send any command while the condition updated==true is on
//
//      Table
//      ---------------------------------------------------------------------------
//      Command       Meaning                  data                    common_data
//       1111         Switch to I/Q mode       N/A                     N/A
//       2222         Switch to Freq & A mode  N/A                     N/A
//       3333         Set drive level          Drive level {0..7}      N/A
//       4444         Change Frequency         Frequency in Hz         N/A
//      ---------------------------------------------------------------------------
//
// Note: with current level if shared memory is enabled the data incoming thru standar input
// or file is expected to be in float format. Other data formats aren't supported together with
// commands, for them sendiq will work but no commands will be honored.
//
// *--- Memory structure

struct shared_memory_struct {
        bool  updated;
        int   command;
        float data;
	char  common_data[1024];
};

bool  running=true;
bool  fdds=false;            //operate as a DDS
float drivedds=0.1;          //drive level

#define PROGRAM_VERSION "0.2"

// *----- Shared memory Variables

      void    *pshared = (void *)0;	      //pointer to IPC structure
      struct shared_memory_struct *sharedmem; //Actual shared memory area
      int    sharedmem_id;                    //token is an unique ID for shared memory
      int    sharedmem_token=0;               //key ID, =0 means no shared memory
// *----- End of shared memory  definitions

void print_usage(void)
{

fprintf(stderr,\
"\nsendiq -%s\n\
Usage:\nsendiq [-i File Input][-s Samplerate][-l] [-f Frequency] [-h Harmonic number] \n\
-i            path to File Input \n\
-s            SampleRate 10000-250000 \n\
-f float      central frequency Hz(50 kHz to 1500 MHz),\n\
-m int        shared memory token,\n\
-d            dds mode,\n\
-p            power level (0.00 to 7.00),\n\
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
  char * endptr = NULL;
	while(1)
	{
		a = getopt(argc, argv, "i:f:s:m:p:h:ldt:");
	
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
		case 'd': // dds mode
			fdds=true;
			break;
		case 'p': // driver level (power)
			drivedds=atof(optarg);
			if (drivedds<0.0) {drivedds=0.0;}
			if (drivedds>7.0) {drivedds=7.0;}
			break;
		case 'f': // Frequency
      SetFrequency = strtof(optarg, &endptr);
      if (endptr == optarg || SetFrequency <= 0.0f || SetFrequency == HUGE_VALF) {
        fprintf(stderr, "tune: not a valid frequency - '%s'", optarg);
        exit(1);
      }
			break;			
    case 'm': // Shared memory token
      sharedmem_token = atoi(optarg);
     	InputType=typeiq_float;      //if using shared memory force float pipe
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
		case 'h': // Harmonic numebr
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

        if (fdds==true) {           //if instructed to operate as DDS start with carrier, otherwise I/Q mode it is
           iqtest.ModeIQ=MODE_FREQ_A;
        }

	//iqtest.print_clock_tree();
	//iqtest.SetPLLMasterLoop(5,6,0);
//=========================================================================================================
// *------ Shared memory definitions if enabled
//=========================================================================================================
   if (sharedmem_token != 0) {
       sharedmem_id = shmget((key_t)sharedmem_token, sizeof(struct shared_memory_struct), 0666 | IPC_CREAT);		//<<<<< SET THE SHARED MEMORY KEY    (Shared memory key , Size in bytes, Permission flags)
       if (sharedmem_id == -1) {
           printf("Shared memory shmget() failed\n");
           exit(8);
       }

// *----- Make the shared memory accessible to the program
       pshared = shmat(sharedmem_id, (void *)0, 0);
       if (pshared == (void *)-1) 	{
          printf("Shared memory shmat() failed\n");
          exit(16);
       }

// *----- Assign the shared_memory segment
       sharedmem = (struct shared_memory_struct *)pshared;

       sharedmem->updated=false;
       sharedmem->command=0x00;
       sharedmem->data=0;
   }
//=========================================================================================================

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
					if(nbread>0)
					{
						for(int i=0;i<nbread/2;i++)
						{

// *-----------------------------------------------------[Patch to introduce IPC handling]-------------------------------------------------------
// * this modification operates only when using the float stream for a  minimum intervention into the original sendiq
// * for consistency and transparence it should be propagated to all pipe modes eventually
// *---------------------------------------------------------------------------------------------------------------------------------------------
           						if (sharedmem_token != 0) {
							   if (sharedmem->updated==true) {
							      if (sharedmem->command == 1111) {
								  iqtest.ModeIQ=MODE_IQ;
								  printf("MODE_IQ selected\n");
 							      }
							      if (sharedmem->command == 2222) {
							  	  iqtest.ModeIQ=MODE_FREQ_A;
						                  printf("MODE_FREQ_A selected\n");
							      }

							      if (sharedmem->command == 3333) {
								  drivedds=sharedmem->data;
								  printf("Drive level set (%f)\n",drivedds);
							      }
							      if (sharedmem->command == 4444) {
								  SetFrequency=sharedmem->data;
								  printf("Frequency set %f\n",SetFrequency);
					                          iqtest.clkgpio::disableclk(4);
					                          iqtest.clkgpio::SetAdvancedPllMode(true);
					                          iqtest.clkgpio::SetCenterFrequency(SetFrequency,SampleRate);
					                          iqtest.clkgpio::SetFrequency(0);
					                          iqtest.clkgpio::enableclk(4);
							      }
							      sharedmem->updated=false;
    							   }
                                                        }
							if (iqtest.ModeIQ==MODE_FREQ_A) {  //if into Frequency-Amplitude mode then only drive a constant carrier
                             				    IQBuffer[i*2]=10.0;            //should be 10 Hz 
                                                            IQBuffer[i*2+1]=drivedds;      //at the defined drive level
                          				}
// *---------------------------------------------------------------------------------------------------------------------------------------------
							if(i%Decimation==0)
							{
								CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2],IQBuffer[i*2+1]);
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

// *--- Detach and delete shared memory
        if (sharedmem_token != 0) {
           if (shmdt(pshared) == -1) {
               printf("shmdt failed\n");
           } else {
             if (shmctl(sharedmem_id, IPC_RMID, 0) == -1) {
        	 printf("shmctl(IPC_RMID) failed\n");
             }
           }
        }
// *--- end of shared memory detach 
}	

