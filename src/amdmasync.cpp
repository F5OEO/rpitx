
#include "stdio.h"
#include "amdmasync.h"
#include <math.h>


amdmasync::amdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize):bufferdma(Channel,FifoSize,3,2)
{

	
	tunefreq=TuneFrequency;
	clkgpio::SetAdvancedPllMode(true);
	clkgpio::SetCenterFrequency(TuneFrequency,SampleRate); 
	clkgpio::SetFrequency(0);
	clkgpio::enableclk(4); // GPIO 4 CLK by default
	syncwithpwm=false;
	
	if(syncwithpwm)
	{
		pwmgpio::SetPllNumber(clk_plld,1);
		pwmgpio::SetFrequency(SampleRate);
	}
	else
	{
		pcmgpio::SetPllNumber(clk_plld,1);
		pcmgpio::SetFrequency(SampleRate);
	}
	
	padgpio pad;
	Originfsel=pad.gpioreg[PADS_GPIO_0];
   
	SetDmaAlgo();

		
}

amdmasync::~amdmasync()
{
	clkgpio::disableclk(4);
	padgpio pad;
	pad.gpioreg[PADS_GPIO_0]=Originfsel;
}



void amdmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < buffersize; samplecnt++) 
			{ 
			
			//@0				
			//Set Amplitude by writing to PADS	
			cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample]);
			cbp->dst = 0x7E000000+(PADS_GPIO_0<<2)+PADS_GPIO;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);		
			cbp++;

			//@1				
			//Set Amplitude  to FSEL for amplitude=0
			cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample+1]); 
			cbp->dst = 0x7E000000 + (GPFSEL0<<2)+GENERAL_BASE; 				
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1); 
			cbp++;
			
				
			// Delay
			if(syncwithpwm)
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP |BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(DREQ_PWM);
			else
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP |BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(DREQ_PCM_TX);
			cbp->src = mem_virt_to_phys(cbarray); // Data is not important as we use it only to feed the PWM
			if(syncwithpwm)		
				cbp->dst = 0x7E000000 + (PWM_FIFO<<2) + PWM_BASE ;
			else
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

void amdmasync::SetAmSample(uint32_t Index,float Amplitude) //-1;1
{
	Index=Index%buffersize;	

	int IntAmplitude=round(abs(Amplitude)*8.0)-1;
	
	int IntAmplitudePAD=IntAmplitude;
	if(IntAmplitudePAD>7) IntAmplitudePAD=7;
	if(IntAmplitudePAD<0) IntAmplitudePAD=0;
	
	//fprintf(stderr,"Amplitude=%f PAD %d\n",Amplitude,IntAmplitudePAD);
	sampletab[Index*registerbysample]=(0x5A<<24) + (IntAmplitudePAD&0x7) + (1<<4) + (0<<3); // Amplitude PAD

	//sampletab[Index*registerbysample+2]=(Originfsel & ~(7 << 12)) | (4 << 12); //Alternate is CLK
	if(IntAmplitude==-1)
		{
			sampletab[Index*registerbysample+1]=(Originfsel & ~(7 << 12)) | (0 << 12); //Pin is in -> Amplitude 0
		}
		else
		{
			sampletab[Index*registerbysample+1]=(Originfsel & ~(7 << 12)) | (4 << 12); //Alternate is CLK
		}
	

	PushSample(Index);
}


