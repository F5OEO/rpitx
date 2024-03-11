#include <unistd.h>
#include <librpitx/librpitx.h>
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
-p set clock ppm instead of ntp adjust\n\
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
	float SetFrequency = 434e6;
	dbg_setlevel(1);
	bool NotKill=false;
  bool ppmSet = false;
	float ppm = 0.0f;
  char * endptr = NULL;
	while(1)
	{
		a = getopt(argc, argv, "f:ehp:");
	
		if(a == -1) 
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(a)
		{
		case 'f': // Frequency
      SetFrequency = strtof(optarg, &endptr);
      if (endptr == optarg || SetFrequency <= 0.0f || SetFrequency == HUGE_VALF) {
        fprintf(stderr, "tune: not a valid frequency - '%s'", optarg);
        exit(1);
      }
			break;
		case 'e': //End immediately
			NotKill=true;
			break;
		case 'p': //ppm
      ppm = strtof(optarg, &endptr);
      if (endptr == optarg || ppm <= 0.0f || ppm == HUGE_VALF) {
        fprintf(stderr, "tune: not a valid ppm - '%s'", optarg);
        exit(1);
      }
      ppmSet = true;
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
		if(ppmSet)	//ppm is set else use ntp
			clk->Setppm(ppm);
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

