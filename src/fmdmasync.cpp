#include "stdio.h"
#include "fmdmasync.h"
#include "gpio.h" //for definition of registers

fmdmasync::fmdmasync(int Channel,uint32_t FifoSize):dma(Channel,FifoSize*2,FifoSize)
{
	SetDmaAlgo();
	FillMemory(12,1472);
}

fmdmasync::~fmdmasync()
{
}

void fmdmasync::SetDmaAlgo()
{
			dma_cb_t *cbp = cbarray;
			for (uint32_t samplecnt = 0; samplecnt < cbsize/2; samplecnt++) { //cbsize/2 because we have 2 CB by sample
			
		
			// Write a frequency sample

			cbp->info = BCM2708_DMA_NO_WIDE_BURSTS /* BCM2708_DMA_WAIT_RESP |BCM2708_DMA_D_DREQ | BCM2708_DMA_PER_MAP(5)*/;
			cbp->src = mem_virt_to_phys(&usermem[samplecnt]);
			cbp->dst = 0x7E000000 | GPCLK_DIV | CLK_BASE ;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			//printf("cbp : sample %x src %x dest %x next %x\n",ctl->sample + i,cbp->src,cbp->dst,cbp->next);
			cbp++;
			
					
			// Delay
			
			cbp->info =  BCM2708_DMA_SRC_IGNOR  |/* BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP |*/ BCM2708_DMA_D_DREQ  | BCM2708_DMA_PER_MAP(2);
			cbp->src = mem_virt_to_phys(usermem); // Data is not important as we use it only to feed the PWM
			cbp->dst = 0x7E000000 | PWM_FIFO | PWM_BASE ;
			cbp->length = 4;
			cbp->stride = 0;
			cbp->next = mem_virt_to_phys(cbp + 1);
			cbp++;
		}
					
		cbp--;
		cbp->next = mem_virt_to_phys(cbarray); // We loop to the first CB
}

void fmdmasync::FillMemory(uint32_t FreqDivider,uint32_t FreqFractionnal)
{
	
	for (uint32_t samplecnt = 0; samplecnt < usermemsize; samplecnt++)
	{
		usermem[samplecnt]=0x5A000000 | ((FreqDivider)<<12) | FreqFractionnal;
		FreqFractionnal=(FreqFractionnal+1)%4096;
	}
}
