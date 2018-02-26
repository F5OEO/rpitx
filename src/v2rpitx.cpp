#include <unistd.h>
#include "dma.h"
#include "gpio.h"

int main(int argc, char* argv[])
{
    clkgpio clk;
	//clk.print_clock_tree();
	clk.SetPllNumber(clk_plld,1);
	
	generalgpio generalio;
	generalio.enableclk();
	for(int i=0;i<100;i++)
	{
		usleep(40000);
		clk.SetFrequency(89000000+i*40);
	}
}
