

#include "stdio.h"
#include "serialdmasync.h"


serialdmasync::serialdmasync(uint32_t SampleRate,int Channel,uint32_t FifoSize,bool dualoutput):bufferdma(Channel,FifoSize,1,1)
{
	if(dualoutput) //Fixme if 2pin we want maybe 2*SRATE as it is distributed over 2 pin
	{
		pwmgpio::SetMode(pwm2pin);
		SampleRate*=2;
	}
	else
	{
		pwmgpio::SetMode(pwm1pin);
	}

	if(SampleRate>250000)
	{
		pwmgpio::SetPllNumber(clk_plld,1);
		pwmgpio::SetFrequency(SampleRate);
	}
	else
	{
		pwmgpio::SetPllNumber(clk_osc,1);
		pwmgpio::SetFrequency(SampleRate);
	}

	enablepwm(12,0); // By default PWM on GPIO 12/pin 32	
    enablepwm(13,0); // By default PWM on GPIO 13/pin 33	

	SetDmaAlgo();

	
	// Note : Spurious are at +/-(19.2MHZ/2^20)*Div*N : (N=1,2,3...) So we need to have a big div to spurious away BUT
	// Spurious are ALSO at +/-(19.2MHZ/2^20)*(2^20-Div)*N
	// Max spurious avoid is to be in the center ! Theory shoud be that spurious are set away at 19.2/2= 9.6Mhz ! But need to get account of div of PLLClock
	
}

serialdmasync::~serialdmasync()
{
}


void serialdmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < buffersize; samplecnt++) 
			{ 
			
							
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP |BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(DREQ_PWM);
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample]); 
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

void serialdmasync::SetSample(uint32_t Index,int Sample)
{
	Index=Index%buffersize;	
	sampletab[Index]=Sample;
	
	PushSample(Index);
}


