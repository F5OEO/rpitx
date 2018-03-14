#ifndef DEF_AMDMASYNC
#define DEF_AMDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class amdmasync:public bufferdma,public clkgpio,public pwmgpio,public pcmgpio
{
	protected:
	uint64_t tunefreq;
	bool syncwithpwm;
	uint32_t Originfsel;
	public:
	amdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize);
	~amdmasync();
	void SetDmaAlgo();
		
	void SetAmSample(uint32_t Index,float Amplitude);
}; 

#endif
