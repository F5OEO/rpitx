#include <unistd.h>
#include "../librpitx/src/librpitx.h"
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>

// DVBS ENCODER
extern "C" 
{
	#include "fec100.h"
}
extern "C" void 	dvbsenco_init	(void) ;
extern "C" uchar*	dvbsenco	(uchar*) ;	
extern "C" void energy (uchar* input,uchar *output) ;
extern "C" void reed (uchar *input188) ;
extern "C" uchar*	interleave 	(uchar* packetin) ; 

// DVBS2 ENCODER
extern "C" uint32_t _dvbs2arm_control (uint32_t command, uint32_t param1) ;
extern "C" uchar* _dvbs2arm_process_packet (uchar *packet188) ;

bool running=true;

#define PROGRAM_VERSION "0.1"

void SimpleTestFileIQ(uint64_t Freq)
{
	
}

void print_usage(void)
{

fprintf(stderr,\
"\ndvbrf -%s\n\
Usage:\ndvbrf [-i File Input][-s Samplerate][-l] [-f Frequency] [-h Harmonic number] \n\
-i            path to Transport Stream File Input \n\
-s            SampleRate 10000-250000 \n\
-c            Fec : {1/2,3/4,5/6,7/8} \n\
-f float      central frequency Hz(50 kHz to 1500 MHz),\n\
-l            loop mode for file input\n\
-n            Use harmonic number n\n\
-?            help (this help).\n\
\n",\
PROGRAM_VERSION);

} /* end function print_usage */

static void
terminate(int num)
{
    
	if(running)
		fprintf(stderr,"Caught signal - Terminating %x\n",num);
	running=false;	
   
}

#define MAX_SAMPLERATE 2000000

int main(int argc, char* argv[])
{
	int a;
	int anyargs = 1;
	float SetFrequency=434e6;
	int SampleRate=48000;
	bool loop_mode_flag=false;
	char* FileName=NULL;
	int Harmonic=1;
	enum {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double};
	int InputType=typeiq_i16;
	int Decimation=1;
	int FEC=-1;
	enum {DVBS,DVBS2};
	int DVB_Mode=DVBS;//
	while(1)
	{
		a = getopt(argc, argv, "i:f:s:n:lt:c:m:");
	
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
				
				
					 fprintf(stderr,"SampleRate too high : >%d sample/s",MAX_SAMPLERATE);
					 exit(1);
						
			};	
			break;
		case 'c': // FEC
			if(strcmp("1/2",optarg)==0) FEC=1;
			if(strcmp("2/3",optarg)==0) FEC=2;	
			if(strcmp("3/4",optarg)==0) FEC=3;
			if(strcmp("5/6",optarg)==0) FEC=5;
			if(strcmp("7/8",optarg)==0) FEC=7;
			if(strcmp("carrier",optarg)==0) {fprintf(stderr,"Carrier mode\n");FEC=0;}//CARRIER MODE
			
			break;
		case 'm': // FEC
			if(strcmp("dvbs",optarg)==0) DVB_Mode=DVBS;
			if(strcmp("dvbs2",optarg)==0) DVB_Mode=DVBS2;
		break;		
		case 'n': // help
			Harmonic=atoi(optarg);
			break;
		case 'l': // loop mode
			loop_mode_flag = true;
			break;
		case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 			{
 				fprintf(stderr, "dvbrf: unknown option `-%c'.\n", optopt);
 			}
			else
			{
				fprintf(stderr, "dvbrf: unknown option character `\\x%x'.\n", optopt);
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
	if(FEC<0)  {fprintf(stderr,"Need a FEC\n");exit(1);}
	 for (int i = 0; i < 0xb; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	FILE *tsfile=NULL;
	if(strcmp(FileName,"-")==0)
		tsfile=fopen("/dev/stdin","rb");
	else	
		tsfile=fopen(FileName,"rb");
	if (tsfile==NULL) 
	{	
		printf("input file issue\n");
		exit(0);
	}

	if(DVB_Mode==DVBS)
	{
		int FifoSize=204*100*4;
		int NumberofPhase=4;
		phasedmasync dvbsmodul(SetFrequency/Harmonic,SampleRate,NumberofPhase,14,FifoSize);
		padgpio pad;
		pad.setlevel(7);

		#define BURST_PACKET 10
		#define BURST_SIZE 204*BURST_PACKET
		int PhaseBuffer[BURST_SIZE*4*2]; //4 dibit by byte

		static uint32_t BuffAligned[256*2];
		static uchar	*TsBuffer=(uchar *)BuffAligned ;
		static unsigned char BuffIQ[204*2]; //QPSK 2bit per sample : 4 dibits by byte
		static uchar *pRS204=NULL;
		

		dvbsenco_init() ;
		viterbi_init(FEC);
		int MapDVBS[4]={0,3,1,2};//0,-pi/2,pi/2,pi
		while(running)
		{
			
						
						int nbread=fread(TsBuffer,sizeof(uchar),BURST_PACKET*188,tsfile);
						
						if(nbread>0)
						{
							if(TsBuffer[0]!=0x47) fprintf(stderr,"SYNC ERROR \n");
							for(int p=0;p<BURST_PACKET;p++)
							{	
								pRS204 = dvbsenco (TsBuffer+p*188) ;		
								
								int NbIQOutput=viterbi (pRS204,BuffIQ);
								
								int dibit=0;
								for(int k=0;k<NbIQOutput;k++)
								{
									for(int i=3;i>=0;i--)
									{
												PhaseBuffer[dibit++]=MapDVBS[(BuffIQ[k]>>(i*2))&0x3];
									
									}
									
									
								}	
								dvbsmodul.SetPhaseSamples(PhaseBuffer,NbIQOutput*4);
							}	
						}
						else 
						{
							printf("End of file\n");
							if(loop_mode_flag)
							fseek ( tsfile , 0 , SEEK_SET );
							else
								running=false;
						}
					
		}	
				

		dvbsmodul.stop();
	}
	else
	{
		int FifoSize=546*30*4;
		int NumberofPhase=4;
		phasedmasync dvbs2modul(SetFrequency/Harmonic,SampleRate,NumberofPhase,14,FifoSize);
		padgpio pad;
		pad.setlevel(7);

		 uint32_t BuffAligned[8832];
		 uchar	*IqBuffer=(uchar *)BuffAligned ;
		unsigned char TsBuffer[188];
		uint32_t Result=-1;
		Result=_dvbs2arm_control (1,(uint32_t)IqBuffer) ;     // set DVB-S2 mode by passing buffer address
		
  		Result=_dvbs2arm_control (2,0x34) ;        // set FEC 3/4
		
	    Result=_dvbs2arm_control (3,0x35) ;        // set rolloff 0.35
		
		Result=_dvbs2arm_control (4,0) ;        // Get Efficiency
		printf("Net Bitrate should be %f\n",Result*1e-6*SampleRate);
		int PhaseBuffer[546*30]; //4 dibit by byte
		unsigned char *IQTempFrame=NULL;
		int MapDVBS[4]={0,3,1,2};//0,-pi/2,pi/2,pi
		while(running)
		{
			
						
						int nbread=fread(TsBuffer,sizeof(uchar),188,tsfile);
						if(nbread>0)
						{
							IQTempFrame =_dvbs2arm_process_packet (TsBuffer);
							
							if(IQTempFrame!=NULL)
							{
								uint32_t *IQFrame=(uint32_t*)IQTempFrame;
								int symbol=0;	
								for(int i=0;i<546/2;i++)
								{
										for(int j=31;j>1;j--)
										{
											PhaseBuffer[symbol++]=MapDVBS[(((IQFrame[2*i]>>j)&1)<<1)|(((IQFrame[2*i+1]>>j)&1))];
											//printf("Symbol=%d\n",PhaseBuffer[symbol]);
										}
								}
								//printf("Symbol=%d\n",symbol);
								dvbs2modul.SetPhaseSamples(PhaseBuffer,symbol);
							}
							else
							{
								//printf("more\n");
							}	
						}
						else
						{
							printf("End of file\n");
							if(loop_mode_flag)
							fseek ( tsfile , 0 , SEEK_SET );
							else
								running=false;
						}

		}
		dvbs2modul.stop();				
	}
	
}	

