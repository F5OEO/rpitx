// Inspired by https://github.com/SaucySoliton/PiFmRds/blob/master/src/pi_fm_rds.c


#include "stdio.h"
#include "ngfmdmasync.h"


ngfmdmasync::ngfmdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize):bufferdma(Channel,FifoSize,2,1)
{

	
	tunefreq=TuneFrequency;
	clkgpio::SetAdvancedPllMode(true);
	clkgpio::SetCenterFrequency(TuneFrequency); // Write Mult Int and Frac : FixMe carrier is already there
	

	/*
	double FloatMult=((double)TuneFrequency)/(double)(XOSC_FREQUENCY);
	uint32_t freqctl = FloatMult*((double)(1<<20)) ; 
	int IntMultiply= freqctl>>20; // Need to be calculated to have a center frequency
	freqctl&=0xFFFFF; // Fractionnal is 20bits
	//fprintf(stderr,"freqctl=%d\n",	freqctl);
	uint32_t FracMultiply=freqctl&0xFFFFC; // Check if last 2 bits are really there

	clkgpio::gpioreg[PLLA_CTRL] = (0x5a<<24) | (0x21<<12) | IntMultiply;

	SetClkDivFrac(4,0); // CLK is not divided for now !	
	*/

	pwmgpio::SetPllNumber(clk_osc,1);
	pwmgpio::SetFrequency(SampleRate);
	pwmgpio::SetMode(0);

	
   
	SetDmaAlgo();

	//FillMemory(IntMultiply,FracMultiply);

	// Note : Spurious are at +/-(19.2MHZ/2^20)*Div*N : (N=1,2,3...) So we need to have a big div to spurious away BUT
	// Spurious are ALSO at +/-(19.2MHZ/2^20)*(2^20-Div)*N
	// Max spurious avoid is to be in the center ! Theory shoud be that spurious are set away at 19.2/2= 9.6Mhz ! But need to get account of div of PLLClock
	
}

ngfmdmasync::~ngfmdmasync()
{
}

void ngfmdmasync::SetPhase(bool inversed)
{
	clkgpio::SetPhase(inversed);
}

void ngfmdmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < buffersize; samplecnt++) 
			{ 
			
			// Write INT Mult
			/*
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample+1]);
			cbp->dst = 0x7E000000 + (PLLA_CTRL<<2) + CLK_BASE ; 
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//fprintf(stderr,"cbp : sample %x src %x dest %x next %x\n",samplecnt,cbp->src,cbp->dst,cbp->next);
			cbp++;
			*/

			// Write a frequency sample

			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample]);
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

void ngfmdmasync::SetFrequencySample(uint32_t Index,int Frequency)
{
	sampletab[Index]=(0x5A<<24)|GetMasterFrac(Frequency);
	//fprintf(stderr,"Frac=%d\n",GetMasterFrac(Frequency));
	PushSample(Index);
}

void ngfmdmasync::FillMemory(uint32_t MultInt,uint32_t FirstFrac)
{
	
	//if(FirstFrac<usermemsize) 	FirstFrac=usermemsize;
	uint32_t tempmul=MultInt;
	for (uint32_t samplecnt = 0; samplecnt < usermemsize/2; samplecnt++)
	{
		//uint32_t Frac=(FirstFrac+samplecnt*((samplecnt%2==0)?1:-1))&0xFFFFF;
		uint32_t Frac=(FirstFrac+samplecnt/10)%(1<<20); //10times less than symbolrate
		//fprintf(stderr,"Frac %d\n",Frac);
		if(Frac==0) 
		{
			fprintf(stderr,"Cross Int\n");
			tempmul=MultInt+1;
		}
		//usermem[samplecnt*2]=(0x5a<<24) | (0x21<<12) | tempmul;
		//usermem[samplecnt*2+1]=0x5A000000 | Frac;
		usermem[samplecnt*2]=0x5A000000 | FirstFrac;
	}
	
}
