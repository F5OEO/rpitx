#include <unistd.h>
#include "librpitx/src/librpitx.h"
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>

bool running=true;

#define PROGRAM_VERSION "0.1"


void print_usage(void)
{

fprintf(stderr,\
"\ntune -%s\n\
Usage:\ntune  [-f Frequency] [-h] \n\
-f float      frequency carrier Hz(50 kHz to 1500 MHz),\n\
-e exit immediately without killing the carrier,\n\
-h            help (this help).\n\
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
	int anyargs = 0;
	float SetFrequency=434e6;
	
	bool NotKill=false;
	while(1)
	{
		a = getopt(argc, argv, "f:eh");
	
		if(a == -1) 
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(a)
		{
		case 'f': // Frequency
			SetFrequency = atof(optarg);
			break;
		case 'e': // SampleRate (Only needeed in IQ mode)
			NotKill=true;
			break;
		case 'h': // help
			print_usage();
			exit(1);
			break;
		case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 			{
 				fprintf(stderr, "tune: unknown option `-%c'.\n", optopt);
 			}
			else
			{
				fprintf(stderr, "tune: unknown option character `\\x%x'.\n", optopt);
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

		generalgpio gengpio;
		gengpio.setpulloff(4);
		padgpio pad;
		pad.setlevel(7);
		clkgpio *clk=new clkgpio;
		clk->SetAdvancedPllMode(true);
		clk->SetCenterFrequency(SetFrequency,10);
		clk->SetFrequency(000);
		clk->enableclk(4);
		//clk->enableclk(6);//CLK2 : experimental
		//clk->enableclk(20);//CLK1 duplicate on GPIO20 for more power ?
		if(!NotKill)
		{
			while(running)
			{
				sleep(1);
			}
			clk->disableclk(4);
			clk->disableclk(20);
			delete(clk);
		}
		else
		{
			//Ugly method : not destroying clk object will not call destructor thus leaving clk running 
		}
	
	
}	

