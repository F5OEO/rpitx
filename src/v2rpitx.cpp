#include <unistd.h>
#include "dma.h"
#include "gpio.h"
#include "fmdmasync.h"
#include "stdio.h"

int main(int argc, char* argv[])
{
    clkgpio clk;
	//clk.print_clock_tree();
	clk.SetPllNumber(clk_plld,1);
	
	generalgpio generalio;
	generalio.enableclk();

	pwmgpio pwm;
	pwm.SetPllNumber(clk_osc,1);
	pwm.SetFrequency(10000);
	pwm.SetMode(0);
	//clk.SetFrequency(89100000);
	
	fmdmasync fmtest(14,400);
	fmtest.start();
	for(int i=0;i<10000;i++)
	{
		
	}	
	sleep(10); 
	fmtest.stop();
	
	/*
	{
		for(int i=0;i<10000;i++)
		{
			usleep(50);
			clk.SetFrequency(89000000+(i%2)*200000);
		}
	}*/
}
