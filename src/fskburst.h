#ifndef DEF_FSKBURST
#define DEF_FSKBURST

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class fskburst:public bufferdma,public clkgpio,public pwmgpio,public pcmgpio
{
	protected:
	uint32_t freqdeviation;
	uint32_t Originfsel;
	bool syncwithpwm;
	dma_cb_t *lastcbp;
	public:
	fskburst(uint64_t TuneFrequency,uint32_t SymbolRate,uint32_t Deviation,int Channel,uint32_t FifoSize);
	~fskburst();
	void SetDmaAlgo();
	
	void SetSymbols(unsigned char *Symbols,uint32_t Size);
}; 

#endif
