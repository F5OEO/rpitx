#include "dma.h"
#include "stdio.h"

extern "C"
{
#include "mailbox.h"
#include "raspberry_pi_revision.h"
}
#include <unistd.h>


#define BUS_TO_PHYS(x) ((x)&~0xC0000000)

dma::dma(int Channel,uint32_t CBSize,uint32_t UserMemSize) // Fixme! Need to check to be 256 Aligned for UserMem
{ 
	fprintf(stderr,"Channel %d CBSize %d UsermemSize %d\n",Channel,CBSize,UserMemSize);
	
	channel=Channel;
    mbox.handle = mbox_open();
	if (mbox.handle < 0)
	{
		fprintf(stderr,"Failed to open mailbox\n");
		
	}
	cbsize=CBSize;
	usermemsize=UserMemSize;

	GetRpiInfo(); // Fill mem_flag and dram_phys_base
    uint32_t MemoryRequired=CBSize*sizeof(dma_cb_t)+UserMemSize*sizeof(uint32_t);
    int NumPages=(MemoryRequired/PAGE_SIZE)+1;
    fprintf(stderr,"%d Size NUM PAGES %d PAGE_SIZE %d\n",MemoryRequired,NumPages,PAGE_SIZE);
	mbox.mem_ref = mem_alloc(mbox.handle, NumPages* PAGE_SIZE, PAGE_SIZE, mem_flag);
	/* TODO: How do we know that succeeded? */
	//fprintf(stderr,"mem_ref %x\n", mbox.mem_ref);
	mbox.bus_addr = mem_lock(mbox.handle, mbox.mem_ref);
	//fprintf(stderr,"bus_addr = %x\n", mbox.bus_addr);
	mbox.virt_addr = (uint8_t *)mapmem(BUS_TO_PHYS(mbox.bus_addr), NumPages* PAGE_SIZE);
	//fprintf(stderr,"virt_addr %p\n", mbox.virt_addr);
	virtbase = (uint8_t *)((uint32_t *)mbox.virt_addr);
	//fprintf(stderr,"virtbase %p\n", virtbase);
    cbarray = (dma_cb_t *)virtbase; // We place DMA Control Blocks (CB) at beginning of virtual memory
	//fprintf(stderr,"cbarray %p\n", cbarray);
    usermem= (unsigned int *)(virtbase+CBSize*sizeof(dma_cb_t)); // user memory is placed after
	//fprintf(stderr,"usermem %p\n", usermem);

	dma_reg.gpioreg[DMA_CS+channel*0x40] = BCM2708_DMA_RESET|DMA_CS_INT; // Remove int flag
	usleep(100);
	dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]=mem_virt_to_phys((void*)cbarray ); // reset to beginning 
}

void dma::GetRpiInfo()
{
	RASPBERRY_PI_INFO_T info;
	if (getRaspberryPiInformation(&info) > 0)
	{
		if(info.peripheralBase==RPI_BROADCOM_2835_PERIPHERAL_BASE)
		{
			
			 dram_phys_base   =  0x40000000;
			 mem_flag         =  MEM_FLAG_L1_NONALLOCATING|MEM_FLAG_HINT_PERMALOCK|MEM_FLAG_NO_INIT;//0x0c;
		}

		if((info.peripheralBase==RPI_BROADCOM_2836_PERIPHERAL_BASE)||(info.peripheralBase==RPI_BROADCOM_2837_PERIPHERAL_BASE))
		{
			
			 dram_phys_base   =  0xc0000000;
			 mem_flag         =  MEM_FLAG_L1_NONALLOCATING/*MEM_FLAG_DIRECT*/|MEM_FLAG_HINT_PERMALOCK|MEM_FLAG_NO_INIT;//0x04;
		}
	}
	else
	{
		fprintf(stderr,"Unknown Raspberry architecture\n");
	}
}

dma::~dma()
{
		/*
        unmapmem(mbox.virt_addr, NumPages * PAGE_SIZE);
		*/
		mem_unlock(mbox.handle, mbox.mem_ref);
		
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
	dma_reg.gpioreg[DMA_CS+channel*0x40] = BCM2708_DMA_RESET;
	usleep(100);
	dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]=mem_virt_to_phys((void*)cbarray ); // reset to beginning 
	dma_reg.gpioreg[DMA_DEBUG+channel*0x40] = 7; // clear debug error flags
	usleep(100);
	dma_reg.gpioreg[DMA_CS+channel*0x40] = DMA_CS_PRIORITY(15) | DMA_CS_PANIC_PRIORITY(15) | DMA_CS_DISDEBUG |DMA_CS_ACTIVE;
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

int dma::getcbposition()
{
	volatile uint32_t dmacb=(uint32_t)(dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]);
	//fprintf(stderr,"cb=%x\n",dmacb);
	if(dmacb>0)
		return mem_phys_to_virt(dmacb)-(uint32_t)virtbase;
	else
		return -1;
	// dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]-mem_virt_to_phys((void *)cbarray );
}

bool dma::isrunning()
{
	 return ((dma_reg.gpioreg[DMA_CS+channel*0x40]&DMA_CS_ACTIVE)>0);
}

bool dma::isunderflow()
{
	//if((dma_reg.gpioreg[DMA_CS+channel*0x40]&DMA_CS_INT)>0)	fprintf(stderr,"Status:%x\n",dma_reg.gpioreg[DMA_CS+channel*0x40]);
	 return ((dma_reg.gpioreg[DMA_CS+channel*0x40]&DMA_CS_INT)>0);
}

//**************************************** BUFFER DMA ********************************************************
bufferdma::bufferdma(int Channel,uint32_t tbuffersize,uint32_t tcbbysample,uint32_t tregisterbysample):dma(Channel,tbuffersize*tcbbysample,tbuffersize*tregisterbysample)
{
	buffersize=tbuffersize;
	cbbysample=tcbbysample;
	registerbysample=tregisterbysample;
	fprintf(stderr,"BufferSize %d , cb %d user %d\n",buffersize,buffersize*cbbysample,buffersize*registerbysample);
	
	

	current_sample=0;
	last_sample=0;
	sample_available=buffersize;

	sampletab=usermem;
}

void bufferdma::SetDmaAlgo()
{
}



int bufferdma::GetBufferAvailable()
{	
	int diffsample=0;
	if(Started)
	{
		int CurrenCbPos=getcbposition();
		if(CurrenCbPos!=-1)
		{
			current_sample=CurrenCbPos/(sizeof(dma_cb_t)*cbbysample);
		}
		else
		{
			fprintf(stderr,"DMA WEIRD STATE\n");
			current_sample=0;
		}
		//fprintf(stderr,"CurrentCB=%d\n",current_sample);
		diffsample=current_sample-last_sample;
		if(diffsample<0) diffsample+=buffersize;

		//fprintf(stderr,"cur %d last %d diff%d\n",current_sample,last_sample,diffsample);
	}
	else
	{
		//last_sample=buffersize-1;
		diffsample=buffersize;
		current_sample=0;
		//fprintf(stderr,"Warning DMA stopped \n");
		//fprintf(stderr,"S:cur %d last %d diff%d\n",current_sample,last_sample,diffsample);
	}
	
	/*
	if(isunderflow())
	{
	fprintf(stderr,"cur %d last %d \n",current_sample,last_sample);	 	
	 fprintf(stderr,"Underflow\n");
	}*/
	
	return diffsample; 
	
}

int bufferdma::GetUserMemIndex()
{
	
	int IndexAvailable=-1;
	//fprintf(stderr,"Avail=%d\n",GetBufferAvailable());
	if(GetBufferAvailable())
	{
		IndexAvailable=last_sample+1;
		if(IndexAvailable>=(int)buffersize) IndexAvailable=0;	
	}
	return IndexAvailable;
}

int bufferdma::PushSample(int Index)
{
	if(Index<0) return -1; // No buffer available

	/*
	dma_cb_t *cbp;
	cbp=&cbarray[last_sample*cbbysample+cbbysample-1];
	cbp->info=cbp->info&(~BCM2708_DMA_SET_INT);
	*/	
	

	last_sample=Index;
	/*	
	cbp=&cbarray[Index*cbbysample+cbbysample-1];
	cbp->info=cbp->info|(BCM2708_DMA_SET_INT);
	*/
	if(Started==false)
	{
		if(last_sample>buffersize/4)
		{
			 start(); // 1/4 Fill buffer before starting DMA
			Started=true;
		}
		
	
	}
	return 0;
	
}
