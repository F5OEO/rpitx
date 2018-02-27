#include <unistd.h>
#include "dma.h"
#include "gpio.h"
#include "fmdmasync.h"

int main(int argc, char* argv[])
{
    clkgpio clk;
	//clk.print_clock_tree();
	clk.SetPllNumber(clk_plld,1);
	
	generalgpio generalio;
	generalio.enableclk();

	pwmgpio pwm;
	pwm.SetPllNumber(clk_plld,1);
	pwm.SetFrequency(1000000);
	pwm.SetMode(0);
	clk.SetFrequency(89000000);

	fmdmasync fmtest(14,4000);
	fmtest.start();
	sleep(20);
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
