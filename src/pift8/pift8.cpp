#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <signal.h>
#include <unistd.h>

#include "ft8_lib/common/wave.h"
#include "ft8_lib/ft8/pack.h"
#include "ft8_lib/ft8/encode.h"
#include "ft8_lib/ft8/constants.h"

#include "../librpitx/src/librpitx.h"

bool running=true;

#define PROGRAM_VERSION "0.1"

void usage() {
    fprintf(stderr,\
"\npift8 -%s\n\
Usage:\npift8  [-m Message][-f Frequency][-p ppm][-o offset][-s slot][-r] [-h] \n\
-m message to transmit (13 caracters)\n\
-f float      frequency carrier Hz(50 kHz to 1500 MHz),\n\
-p set clock ppm instead of ntp adjust\n\
-o set frequency offset(0-2500Hz) default:1240\n\
-s set time slot to transmit 0 or 1 (2 is always)\n\
-r repeat (every 15s)\n\
-h            help (this help).\n\
Example : sudo pift8 -m \"CQ CA0ALL JN06\" -f 14.074e6\n\
\n",\
PROGRAM_VERSION);
}


static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
}

void wait_every(
  int seconds, int slot
) {
  time_t t;
  struct tm* ptm;
  for(;running;){
    time(&t);
    ptm = gmtime(&t);
    if( ((ptm->tm_sec) % seconds)==slot ) break;
    usleep(1000);
  }
  //usleep(400000); // wait another second
}

int main(int argc, char **argv) {
    
   
    int a;
	int anyargs = 0;

    float frequency=14.07e6;
    float ppm=1000;
    const char *message="CQ";
    bool repeat=false;
    int slot=0;
    float offset=1240;
    float RampRatio=0;
    while(1)
	{
		a = getopt(argc, argv, "m:f:p:hro:s:e:");
	
		if(a == -1) 
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(a)
		{

        case 'm': // message
			message = optarg;
            fprintf(stderr,"Message %s\n ",message);
			break;
		    
		case 'f': // Frequency
			frequency = atof(optarg);
			break;
		
		case 'p': //ppm
			ppm=atof(optarg);
			break;	
		case 'h': // help
			usage();
			exit(1);
			break;
        case 'r': // repeat
			repeat=true;
			break;
        case 's': // time slot
			slot=atoi(optarg);
            fprintf(stderr,"slot=%d\n",slot);
			break;
        case 'o': // frequency offset
			offset=atof(optarg);
			break;
        case 'e': // Ramp Ratio (0..1)
			RampRatio=atof(optarg);
			break;            
		case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 			{
 				fprintf(stderr, "pift8: unknown option `-%c'.\n", optopt);
 			}
			else
			{
				fprintf(stderr, "pift8: unknown option character `\\x%x'.\n", optopt);
			}
			usage();

			exit(1);
			break;			
		default:
			usage();
			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */
    
    // First, pack the text data into binary message
    uint8_t packed[ft8::K_BYTES];
    //int rc = packmsg(message, packed);
    int rc = ft8::pack77(message, packed);
    if (rc < 0) {
        printf("Cannot parse message!\n");
        printf("RC = %d\n", rc);
        return -2;
    }

    printf("Packed data: ");
    for (int j = 0; j < 10; ++j) {
        printf("%02x ", packed[j]);
    }
    printf("\n");

    // Second, encode the binary message as a sequence of FSK tones
    uint8_t tones[ft8::NN];          // FT8_NN = 79, lack of better name at the moment
    //genft8(packed, 0, tones);
    ft8::genft8(packed, tones);

    printf("FSK tones: ");
    for (int j = 0; j < ft8::NN; ++j) {
        printf("%d", tones[j]);
    }
    printf("\n");

    for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

    
    int Upsample=100;
    int FifoSize=ft8::NN;
    float Deviation=6.25;
    dbg_setlevel(1);

    fskburst fsk(frequency+offset, 6.25, Deviation, 14, FifoSize,Upsample,RampRatio);
    if(ppm!=1000)
    {	//ppm is set else use ntp
			fsk.Setppm(ppm);
            fsk.SetCenterFrequency(frequency,50);            
    }    
	//padgpio pad;
	//pad.setlevel(7);// Set max power

	unsigned char Symbols[FifoSize];
    
    for(size_t i=0;(i<ft8::NN)&&running;i++)
    {
		//for(int j=0;j<Upsample;j++)
             Symbols[i]=tones[i];
        
           
		
	    //fprintf(stderr,"Freq %f\n",Symbols[i]);
    }
    fprintf(stderr,"Wait next slot\n");
    if(slot<2)
        wait_every(30,slot*15);
    else
        wait_every(15,0);
    do
    {
        if(!running) exit(0);
        fprintf(stderr,"Tx!\n");
        fsk.SetSymbols(Symbols, (ft8::NN));
        fsk.stop();
        fprintf(stderr,"End of Tx\n");
        if(repeat)
        {
            fprintf(stderr,"Wait next slot\n");
            if(slot<2)
             wait_every(30,slot*15);
            else
              wait_every(15,0);
        }    
    }  while(repeat&&running);  
}
