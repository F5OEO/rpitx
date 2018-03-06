#ifndef DEF_NGFMDMASYNC
#define DEF_NGFMDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class ngfmdmasync:public bufferdma,public clkgpio,public pwmgpio,public pcmgpio
{
	protected:
	uint64_t tunefreq;
	bool syncwithpwm;
	public:
	ngfmdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize);
	~ngfmdmasync();
	void SetDmaAlgo();
	
	void SetPhase(bool inversed);
	void SetFrequencySample(uint32_t Index,int Frequency);
}; 

#endif
