

#include "stdio.h"
#include "phasedmasync.h"
#include <unistd.h>

phasedmasync::phasedmasync(uint64_t TuneFrequency,uint32_t SampleRate,int NumberOfPhase,int Channel,uint32_t FifoSize):bufferdma(Channel,FifoSize,2,1) // Number of phase between 2 and 16
{
	
	SetMode(pwm1pinrepeat);
	pwmgpio::SetPllNumber(clk_plla,0);

	tunefreq=TuneFrequency*NumberOfPhase;
	
	if((NumberOfPhase==2)||(NumberOfPhase==4)||(NumberOfPhase==8)||(NumberOfPhase==16)||(NumberOfPhase==32))
		NumbPhase=NumberOfPhase;
	else
		fprintf(stderr,"PWM critical error: %d is not a legal number of phase\n",NumberOfPhase);
	clkgpio::SetAdvancedPllMode(true);
	
	clkgpio::ComputeBestLO(tunefreq,0); // compute PWM divider according to MasterPLL clkgpio::PllFixDivider
	double FloatMult=((double)(tunefreq)*clkgpio::PllFixDivider)/(double)(XOSC_FREQUENCY);
	uint32_t freqctl = FloatMult*((double)(1<<20)) ; 
	int IntMultiply= freqctl>>20; // Need to be calculated to have a center frequency
	freqctl&=0xFFFFF; // Fractionnal is 20bits
	uint32_t FracMultiply=freqctl&0xFFFFF; 
	clkgpio::SetMasterMultFrac(IntMultiply,FracMultiply);
	fprintf(stderr,"PWM Mult %d Frac %d Div %d\n",IntMultiply,FracMultiply,clkgpio::PllFixDivider);
	
	
	pwmgpio::clk.gpioreg[PWMCLK_DIV] = 0x5A000000 | ((clkgpio::PllFixDivider)<<12); // PWM clock input divider				
	usleep(100);
	pwmgpio::clk.gpioreg[PWMCLK_CNTL]= 0x5A000000 | (pwmgpio::Mash << 9) | pwmgpio::pllnumber|(1 << 4)  ; //4 is START CLK
	usleep(100);
	pwmgpio::SetPrediv(32);	//SetMode should be called before
	
	

	enablepwm(12,0); // By default PWM on GPIO 12/pin 32	
	

	pcmgpio::SetPllNumber(clk_plld,1);// Clk for Samplerate by PCM
	pcmgpio::SetFrequency(SampleRate);
	
		
   
	SetDmaAlgo();
	
	uint32_t ZeroPhase=0;
	switch(NumbPhase)
	{
		case 2:ZeroPhase=0xAAAAAAAA;break;//1,0,1,0 1,0,1,0 
		case 4:ZeroPhase=0xCCCCCCCC;break;//1,1,0,0 //4
		case 8:ZeroPhase=0xF0F0F0F0;break;//1,1,1,1,0,0,0,0 //8 
		case 16:ZeroPhase=0xFF00FF00;break;//1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0 //16
		case 32:ZeroPhase=0xFFFF0000;break;//1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 //32
		default:fprintf(stderr,"Zero phase not initialized\n");break;
	}
	
	for(int i=0;i<NumbPhase;i++)	
	{
		TabPhase[i]=ZeroPhase;
		fprintf(stderr,"Phase[%d]=%x\n",i,TabPhase[i]);
		ZeroPhase=(ZeroPhase<<1)|(ZeroPhase>>31);
	}

	
}

phasedmasync::~phasedmasync()
{
	disablepwm(12);
}


void phasedmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < buffersize; samplecnt++) 
			{ 
			
							
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample]); 
			cbp->dst = 0x7E000000 + (PWM_FIFO<<2) + PWM_BASE ;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//fprintf(stderr,"cbp : sample %x src %x dest %x next %x\n",samplecnt,cbp->src,cbp->dst,cbp->next);
			cbp++;
			
			
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP |BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(DREQ_PCM_TX);
			cbp->src = mem_virt_to_phys(cbarray); // Data is not important as we use it only to feed the PWM
			cbp->dst = 0x7E000000 + (PCM_FIFO_A<<2) + PCM_BASE ;
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

void phasedmasync::SetPhase(uint32_t Index,int Phase)
{
	Index=Index%buffersize;
	Phase=Phase%NumbPhase;
	sampletab[Index]=TabPhase[Phase];
	PushSample(Index);
	
}


