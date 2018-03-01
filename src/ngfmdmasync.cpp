#include "stdio.h"
#include "ngfmdmasync.h"


ngfmdmasync::ngfmdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize):dma(Channel,FifoSize*2,FifoSize)
{
	tunefreq=TuneFrequency;
	clkgpio::SetPllNumber(clk_plla,0); // Use PPL_A , Do not USE MASH which generates spurious
	clkgpio::gpioreg[0x104/4]=0x5A00020A; // Enable Plla_PER
	clkgpio::gpioreg[PLLA_PER]=0x5A000002; // Div ? 

	double FloatMult=((double)TuneFrequency)/(double)(XOSC_FREQUENCY);
	uint32_t freqctl = FloatMult*((double)(1<<20)) ; 
	int IntMultiply= freqctl>>20; // Need to be calculated to have a center frequency
	freqctl&=0xFFFFF; // Fractionnal is 20bits
	//fprintf(stderr,"freqctl=%d\n",	freqctl);
	uint32_t FracMultiply=freqctl&0xFFFFC; // Check if last 2 bits are really there

	clkgpio::gpioreg[PLLA_CTRL] = (0x5a<<24) | (0x21<<12) | IntMultiply;
	//clkgpio::gpioreg[PLLA_FRAC] = (0x5a<<24) | (0xFFFF);
	SetClkDivFrac(2,0); // CLK is not divided for now !	
	
	pwmgpio::SetPllNumber(clk_osc,1);
	pwmgpio::SetFrequency(SampleRate);
	pwmgpio::SetMode(0);

   //clkgpio::print_clock_tree();
	SetDmaAlgo();
	FillMemory(FracMultiply);
}

ngfmdmasync::~ngfmdmasync()
{
}

void ngfmdmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < cbsize/2; samplecnt++) { //cbsize/2 because we have 2 CB by sample
			
		
			// Write a frequency sample

			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt]);
			cbp->dst = 0x7E000000 + (PLLA_FRAC<<2) + CLK_BASE ; 
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//fprintf(stderr,"cbp : sample %x src %x dest %x next %x\n",samplecnt,cbp->src,cbp->dst,cbp->next);
			cbp++;
			
					
			// Delay
			
			cbp->info =  /*BCM2708_DMA_SRC_IGNOR  | */BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(DREQ_PWM);
			cbp->src = mem_virt_to_phys(cbarray); // Data is not important as we use it only to feed the PWM
			cbp->dst = 0x7E000000 + (PWM_FIFO<<2) + PWM_BASE ;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//fprintf(stderr,"cbp : sample %x src %x dest %x next %x\n",samplecnt,cbp->src,cbp->dst,cbp->next);
			cbp++;
		}
					
		cbp--;
		cbp->next = mem_virt_to_phys(cbarray); // We loop to the first CB
		//fprintf(stderr,"Last cbp :  src %x dest %x next %x\n",cbp->src,cbp->dst,cbp->next);
}

void ngfmdmasync::FillMemory(uint32_t FirstFrac)
{
	if(FirstFrac<usermemsize) 	FirstFrac=usermemsize;
	for (uint32_t samplecnt = 0; samplecnt < usermemsize; samplecnt++)
	{
		uint32_t Frac=(FirstFrac+samplecnt*((samplecnt%2==0)?1:-1))&0xFFFFF;
		
		usermem[samplecnt]=0x5A000000 | Frac;
	}
	
}
