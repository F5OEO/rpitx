// Inspired by https://github.com/SaucySoliton/PiFmRds/blob/master/src/pi_fm_rds.c


#include "stdio.h"
#include "iqdmasync.h"


iqdmasync::iqdmasync(uint64_t TuneFrequency,uint32_t SampleRate,int Channel,uint32_t FifoSize):bufferdma(Channel,FifoSize,4,3)
{
// Usermem :
// FRAC frequency
// PAD Amplitude
// FSEL for amplitude 0
	
	tunefreq=TuneFrequency;
	clkgpio::SetAdvancedPllMode(true);
	clkgpio::SetCenterFrequency(TuneFrequency); // Write Mult Int and Frac : FixMe carrier is already there
	clkgpio::SetFrequency(0);
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
	
	mydsp.samplerate=SampleRate;
   
	padgpio pad;
	Originfsel=pad.gpioreg[PADS_GPIO_0];

	SetDmaAlgo();

	
	// Note : Spurious are at +/-(19.2MHZ/2^20)*Div*N : (N=1,2,3...) So we need to have a big div to spurious away BUT
	// Spurious are ALSO at +/-(19.2MHZ/2^20)*(2^20-Div)*N
	// Max spurious avoid is to be in the center ! Theory shoud be that spurious are set away at 19.2/2= 9.6Mhz ! But need to get account of div of PLLClock
	
}

iqdmasync::~iqdmasync()
{
}

void iqdmasync::SetPhase(bool inversed)
{
	clkgpio::SetPhase(inversed);
}

void iqdmasync::SetDmaAlgo()
{
	dma_cb_t *cbp = cbarray;
	for (uint32_t samplecnt = 0; samplecnt < buffersize; samplecnt++) 
		{ 
			
			//@0				
			//Set Amplitude by writing to PADS	
			cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample+1]);
			cbp->dst = 0x7E000000+(PADS_GPIO_0<<2)+PADS_GPIO;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);		
			cbp++;

			//@1				
			//Set Amplitude  to FSEL for amplitude=0
			cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample+2]); 
			cbp->dst = 0x7E000000 + (GPFSEL0<<2)+GENERAL_BASE; 				
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1); 
			cbp++;

			//@2 Write a frequency sample

			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP ;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt*registerbysample]);
			cbp->dst = 0x7E000000 + (PLLA_FRAC<<2) + CLK_BASE ; 
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//fprintf(stderr,"cbp : sample %x src %x dest %x next %x\n",samplecnt,cbp->src,cbp->dst,cbp->next);
			cbp++;
			
				
			//@3 Delay
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


void iqdmasync::SetIQSample(uint32_t Index,liquid_float_complex sample)
{
	Index=Index%buffersize;	
	mydsp.pushsample(sample);
	/*if(mydsp.frequency>2250) mydsp.frequency=2250;
	if(mydsp.frequency<1000) mydsp.frequency=1000;*/
	sampletab[Index*registerbysample]=(0x5A<<24)|GetMasterFrac(mydsp.frequency); //Frequency
	int IntAmplitude=(int)(mydsp.amplitude*1e4*8.0)-1;

	int IntAmplitudePAD=0;
	if(IntAmplitude>7) IntAmplitudePAD=7;
	if(IntAmplitude<0) IntAmplitudePAD=0;
	sampletab[Index*registerbysample+1]=(0x5A<<24) + (IntAmplitudePAD&0x7) + (1<<4) + (0<<3); // Amplitude PAD

	//sampletab[Index*registerbysample+2]=(Originfsel & ~(7 << 12)) | (4 << 12); //Alternate is CLK
	if(IntAmplitude==-1)
		{
			sampletab[Index*registerbysample+2]=(Originfsel & ~(7 << 12)) | (0 << 12); //Pin is in -> Amplitude 0
		}
		else
		{
			sampletab[Index*registerbysample+2]=(Originfsel & ~(7 << 12)) | (4 << 12); //Alternate is CLK
		}
	
		//fprintf(stderr,"amp%f %d\n",mydsp.amplitude,IntAmplitudePAD);
	PushSample(Index);
}


