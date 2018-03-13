#ifndef DEF_SERIALDMASYNC
#define DEF_SERIALDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class serialdmasync:public bufferdma,public clkgpio,public pwmgpio
{
	protected:
	uint64_t tunefreq;
	bool syncwithpwm;
	public:
	serialdmasync(uint32_t SampleRate,int Channel,uint32_t FifoSize,bool dualoutput);
	~serialdmasync();
	void SetDmaAlgo();
		
	void SetSample(uint32_t Index,int Sample);
};
#endif
