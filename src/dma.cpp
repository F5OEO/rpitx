#include "dma.h"
#include "stdio.h"

extern "C"
{
#include "mailbox.h"
}
#include <unistd.h>


#define BUS_TO_PHYS(x) ((x)&~0xC0000000)

dma::dma(int Channel,int CBSize,int UserMemSize,unsigned int mem_flag)
{
	channel=Channel;
    mbox.handle = mbox_open();
	if (mbox.handle < 0)
	{
		fprintf(stderr,"Failed to open mailbox\n");
		
	}
	
    unsigned int MemoryRequired=CBSize*sizeof(dma_cb_t)+UserMemSize*sizeof(unsigned int);
    int NumPages=(MemoryRequired/PAGE_SIZE)+1;
    fprintf(stderr,"%d Size NUM PAGES %d PAGE_SIZE %d\n",MemoryRequired,NumPages,PAGE_SIZE);
	mbox.mem_ref = mem_alloc(mbox.handle, NumPages* PAGE_SIZE, PAGE_SIZE, mem_flag);
	/* TODO: How do we know that succeeded? */
	//printf("mem_ref %x\n", mbox.mem_ref);
	mbox.bus_addr = mem_lock(mbox.handle, mbox.mem_ref);
	//printf("bus_addr = %x\n", mbox.bus_addr);
	mbox.virt_addr = (uint8_t *)mapmem(BUS_TO_PHYS(mbox.bus_addr), NumPages* PAGE_SIZE);
	//printf("virt_addr %p\n", mbox.virt_addr);
	virtbase = (uint8_t *)((uint32_t *)mbox.virt_addr);
    cbarray = (dma_cb_t *)virtbase; // We place DMA Control Blocks (CB) at beginning of virtual memory
    usermem= (unsigned int *)(virtbase+UserMemSize*sizeof(unsigned int)); // user memory is placed after
}

dma::~dma()
{
        unmapmem(mbox.virt_addr, NumPages * PAGE_SIZE);
		//printf("Unmapmem Done\n");
		mem_unlock(mbox.handle, mbox.mem_ref);
		//printf("Unmaplock Done\n");
		mem_free(mbox.handle, mbox.mem_ref);
}

uint32_t dma::mem_virt_to_phys(volatile void *virt)
{	
	//MBOX METHOD
	uint32_t offset = (uint8_t *)virt - mbox.virt_addr;
	return mbox.bus_addr + offset;
}

uint32_t dma::mem_phys_to_virt(volatile uint32_t phys)
{
	//MBOX METHOD
	uint32_t offset=phys-mbox.bus_addr;
	uint32_t result=(uint32_t)((uint8_t *)mbox.virt_addr+offset);
	//printf("MemtoVirt:Offset=%lx phys=%lx -> %lx\n",offset,phys,result);
	return result;
}

int dma::start()
{
	dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]=mem_virt_to_phys((void*)cbarray ); // reset to beginning 
	usleep(100);
	dma_reg.gpioreg[DMA_CS+channel*0x40] = DMA_CS_PRIORITY(7) | DMA_CS_PANIC_PRIORITY(7) | DMA_CS_DISDEBUG |DMA_CS_ACTIVE;
	return 0;
}

int dma::stop()
{
    dma_reg.gpioreg[DMA_CS+channel*0x40] = BCM2708_DMA_RESET;
	usleep(1000);
	dma_reg.gpioreg[DMA_CS+channel*0x40] = BCM2708_DMA_INT | BCM2708_DMA_END;
	usleep(100);
	dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]=mem_virt_to_phys((void *)cbarray );
	usleep(100);
	dma_reg.gpioreg[DMA_DEBUG+channel*0x40] = 7; // clear debug error flags
	usleep(100);
    return 0;
}

