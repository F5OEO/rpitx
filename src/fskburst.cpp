
#include "stdio.h"
#include "fskburst.h"


	fskburst::fskburst(uint64_t TuneFrequency,uint32_t SymbolRate,uint32_t Deviation,int Channel,uint32_t FifoSize):bufferdma(Channel,FifoSize+2,2,1),freqdeviation(Deviation)
	{
		
		clkgpio::SetAdvancedPllMode(true);
		clkgpio::SetCenterFrequency(TuneFrequency,SymbolRate); // Write Mult Int and Frac : FixMe carrier is already there
		clkgpio::SetFrequency(0);
		//clkgpio::enableclk(4); // GPIO 4 CLK by default
		syncwithpwm=false;
	
		if(syncwithpwm)
		{
			pwmgpio::SetPllNumber(clk_plld,1);
			pwmgpio::SetFrequency(SymbolRate);
		}
		else
		{
			pcmgpio::SetPllNumber(clk_plld,1);
			pcmgpio::SetFrequency(SymbolRate);
		}
	
	
	   
		SetDmaAlgo();

		padgpio pad;
		Originfsel=pad.gpioreg[PADS_GPIO_0];
	}

	fskburst::~fskburst()
	{
	}

	void fskburst::SetDmaAlgo()
{
			sampletab[buffersize*registerbysample-2]=(Originfsel & ~(7 << 12)) | (4 << 12); //Enable Clk
			sampletab[buffersize*registerbysample-1]=(Originfsel & ~(7 << 12)) | (0 << 12); //Disable Clk

			dma_cb_t *cbp = cbarray;
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[buffersize*registerbysample-2]); 
			cbp->dst = 0x7E000000 + (GPFSEL0<<2)+GENERAL_BASE; 				
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1); // Stop DMA
			cbp++;			
			
			for (uint32_t samplecnt = 0; samplecnt < buffersize-2; samplecnt++) 
			{ 
			
			
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
			lastcbp=cbp;
			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
			cbp->src = mem_virt_to_phys(&usermem[(buffersize*registerbysample-1)]); 
			cbp->dst = 0x7E000000 + (GPFSEL0<<2)+GENERAL_BASE; 				
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = 0; // Stop DMA			
		
		//fprintf(stderr,"Last cbp :  src %x dest %x next %x\n",cbp->src,cbp->dst,cbp->next);
}
	void fskburst::SetSymbols(unsigned char *Symbols,uint32_t Size)
	{
		if(Size>buffersize-2) {fprintf(stderr,"Buffer overflow\n");return;}
		
		dma_cb_t *cbp=cbarray+1+1;
		for(int i=0;i<Size;i++)
		{
			sampletab[i]=(0x5A<<24)|GetMasterFrac((Symbols[i]==0)?-freqdeviation:freqdeviation);
			
			cbp = &cbarray[i*cbbysample+1+1];
			cbp->next = mem_virt_to_phys(cbp + 1);
		}
		cbp->next = mem_virt_to_phys(lastcbp);

		
		dma::start();
		
		
	}

