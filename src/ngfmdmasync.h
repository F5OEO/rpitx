#ifndef DEF_NGFMDMASYNC
#define DEF_NGFMDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class ngfmdmasync:public dma,clkgpio,pwmgpio
{
	protected:
	uint64_t tunefreq;
	public:
	ngfmdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize);
	~ngfmdmasync();
	void SetDmaAlgo();
	void FillMemory(uint32_t FirstFrac);
}; 

#endif
