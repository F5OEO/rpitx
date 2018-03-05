#include <unistd.h>
#include "dma.h"
#include "gpio.h"
#include "fmdmasync.h"
#include "ngfmdmasync.h"
#include "stdio.h"

int main(int argc, char* argv[])
{
	
	generalgpio generalio;
	generalio.enableclk();

	/*clkgpio clk;
	clk.SetPllNumber(clk_plld,1);
	clk.SetAdvancedPllMode(true);
	clk.SetCenterFrequency(144100000);
	for(int i=0;i<1000000;i+=10)
	{
		clk.SetFrequency(i);
		usleep(10);
	}
	sleep(5);*/
	
	//dma mydma(14,32,16);
    //bufferdma mydma(14,16,2,1);
	
	ngfmdmasync ngfmtest(144100000,5000,14,16);
	
	for(int i=0;i<10;i++)
	{
		int Index=ngfmtest.GetUserMemIndex();
		if(Index>=0)
		{
			ngfmtest.SetFrequencySample(Index,i);
			
		}
		else
			usleep(100);
	}
	fprintf(stderr,"End\n");
	sleep(10);
	ngfmtest.stop();
		

	
// Test Fmdmasync
/* 
	pwmgpio pwm;
	pwm.SetPllNumber(clk_osc,1);
	pwm.SetFrequency(5000);
	pwm.SetMode(0);
	
	fmdmasync fmtest(14,72);
	fmtest.start();
	for(int i=0;i<10000;i++)
	{
		
	}
	
	sleep(180); 
	fmtest.stop();
*/
	
}	

