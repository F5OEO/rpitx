
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
#include "costas8.h"

float frequency=14.07e6;
bool running=true;

//ngfmdmasync *fmmod;
fskburst *fsk;
static double GlobalTuningFrequency=00000.0;


void Encode(char Symbol,unsigned char *Tab,int Upsample)
{
        for(int i=0;i<8;i++)
        {

            fprintf(stderr,"freq %d -> %c\n",4*(Costas8[Symbol][i]-1),Symbol);
            for(int j=0;j<Upsample;j++)
                Tab[i*Upsample+j]=(Costas8[Symbol][i]-1);
    	    
        }    
}

void wait_every(
  int minute
) {
  time_t t;
  struct tm* ptm;
  for(;running;){
    time(&t);
    ptm = gmtime(&t);
    if((ptm->tm_min % minute) == 0 && ptm->tm_sec == 0) break;
    usleep(1000);
  }
  usleep(1000000); // wait another second
}

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
}


int main(int argc, char **argv)
{
	char *Message="F5OEO";
	int SampleRate=4;
	if (argc >2 ) 
	{
		 frequency=atof(argv[1]);
		Message=argv[2];
	}
	
	if(argc<=2)
	{
		printf("usage : corel8 frequency(Hz) Message\n");
		exit(0);
	}

	for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	
		
	short VCOFreq;
    
    
    int NbSymbol=5;
    
    int SR = 4;
	float Deviation = 4;
    int Upsample=100;
	int FifoSize = 8*(20+1); //8 symbols * 20 caracters max, 1 Symbol SYNC
	fsk=new fskburst(frequency, SR, Deviation, 14, FifoSize,Upsample,0.4);

	unsigned char TabSymbol[FifoSize];
    NbSymbol=strlen(Message);
	fprintf(stderr,"Nb Symbols=%d\n",NbSymbol);
    dbg_setlevel(1);
    while(running)
    {
        fprintf(stderr,"Wait next minute\n");
        wait_every(1);
        if(!running) break;
        fprintf(stderr,"Begin Tx\n");
        unsigned char *TabChar=TabSymbol;        
        Encode(1,TabChar,1);
        TabChar+=8;    
        for(int symbol=0;(symbol<NbSymbol)&&running;symbol++)
        {
            Encode(Message[symbol],TabChar,1);
            TabChar+=8;
        }
        fsk->SetSymbols(TabSymbol, (NbSymbol+1)*8);
       fsk->stop();
    }    
	delete fsk;
	return 0;
}
	



