#ifndef DEF_IQDMASYNC
#define DEF_IQDMASYNC

#include "stdint.h"
#include "dma.h"
#include "gpio.h"
#include "dsp.h"
#include <liquid/liquid.h>


class iqdmasync:public bufferdma,public clkgpio,public pwmgpio,public pcmgpio
{
	protected:
	uint64_t tunefreq;
	bool syncwithpwm;
	dsp mydsp;
	uint32_t	Originfsel; //Save the original FSEL GPIO
	public:
	iqdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize);
	~iqdmasync();
	void SetDmaAlgo();
	
	void SetPhase(bool inversed);
	void SetIQSample(uint32_t Index,liquid_float_complex sample);
}; 

#endif
