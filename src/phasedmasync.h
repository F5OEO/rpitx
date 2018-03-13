#ifndef DEF_PHASEDMASYNC
#define DEF_PHASEDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"

class phasedmasync:public bufferdma,public clkgpio,public pwmgpio,public pcmgpio,public generalgpio
{
	protected:
	uint64_t tunefreq;
	int NumbPhase=2;

	uint32_t TabPhase[32];//32 is Max Phase
	public:
	phasedmasync(uint64_t TuneFrequency,uint32_t SampleRate,int NumberOfPhase,int Channel,uint32_t FifoSize);
	~phasedmasync();
	void SetDmaAlgo();
	void SetPhase(uint32_t Index,int Phase);
	
};
#endif
