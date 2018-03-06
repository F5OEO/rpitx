#ifndef DEF_DMA
#define DEF_DMA
#include "stdint.h"
#include "gpio.h"

// ---- Memory allocating defines
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define MEM_FLAG_DISCARDABLE  (1 << 0) /* can be resized to 0 at any time. Use for cached data */
#define MEM_FLAG_NORMAL  (0 << 2) /* normal allocating alias. Don't use from ARM */
#define MEM_FLAG_DIRECT  (1 << 2) /* 0xC alias uncached */
#define MEM_FLAG_COHERENT  (2 << 2) /* 0x8 alias. Non-allocating in L2 but coherent */
#define MEM_FLAG_L1_NONALLOCATING  (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT) /* Allocating in L2 */
#define MEM_FLAG_ZERO  ( 1 << 4)  /* initialise buffer to all zeros */
#define MEM_FLAG_NO_INIT  ( 1 << 5) /* don't initialise (default is initialise to all ones */
#define MEM_FLAG_HINT_PERMALOCK  (1 << 6) /* Likely to be locked for long periods of time. */

#define BCM2708_DMA_SRC_IGNOR           (1<<11)
#define BCM2708_DMA_SRC_INC		(1<<8)
#define BCM2708_DMA_DST_IGNOR           (1<<7)
#define BCM2708_DMA_NO_WIDE_BURSTS	(1<<26)
#define BCM2708_DMA_WAIT_RESP		(1<<3)


#define BCM2708_DMA_D_DREQ		(1<<6)
#define BCM2708_DMA_PER_MAP(x)		((x)<<16)
#define BCM2708_DMA_END			(1<<1)
#define BCM2708_DMA_RESET		(1<<31)
#define BCM2708_DMA_INT			(1<<2)

#define DMA_CS			(0x00/4)
#define DMA_CONBLK_AD		(0x04/4)
#define DMA_DEBUG		(0x20/4)

//Page 61
#define DREQ_PCM_TX 2
#define DREQ_PCM_RX 3
#define DREQ_SMI 4
#define DREQ_PWM 5
#define DREQ_SPI_TX 6
#define DREQ_SPI_RX 7
#define DREQ_SPI_SLAVE_TX 8
#define DREQ_SPI_SLAVE_RX 9


class dma
{
    protected:
    struct {
	    int handle;		/* From mbox_open() */
	    unsigned mem_ref;	/* From mem_alloc() */
	    unsigned bus_addr;	/* From mem_lock() */
	    uint8_t *virt_addr;	/* From mapmem() */
    } mbox;

    typedef struct {
	uint32_t info, src, dst, length,
		 stride, next, pad[2];
    } dma_cb_t; //8*4=32 bytes

    typedef struct {
	uint8_t *virtaddr;
	uint32_t physaddr;
    } page_map_t;

    page_map_t *page_map;

    uint8_t *virtbase;
    int NumPages=0;
	int channel; //DMA Channel
	dmagpio dma_reg;

	uint32_t mem_flag; //Cache or not depending on Rpi1 or 2/3
	uint32_t dram_phys_base;

		
    public:
    dma_cb_t *cbarray;
	uint32_t cbsize;
    uint32_t *usermem;
	uint32_t usermemsize;
	bool Started=false;

    dma(int Channel,uint32_t CBSize,uint32_t UserMemSize);
    ~dma();
    uint32_t mem_virt_to_phys(volatile void *virt);
    uint32_t mem_phys_to_virt(volatile uint32_t phys);
	void GetRpiInfo();
    int start();
    int stop();
	int getcbposition();
	bool isrunning();
	bool isunderflow();
	    
};

#define PHYSICAL_BUS 0x7E000000

class bufferdma:public dma
{
	protected:
	

	uint32_t current_sample;
	uint32_t last_sample;
	uint32_t sample_available;
	
	public:
	uint32_t buffersize;
	uint32_t cbbysample;
	uint32_t registerbysample;
	uint32_t *sampletab;
	
	public:
	bufferdma(int Channel,uint32_t tbuffersize,uint32_t tcbbysample,uint32_t tregisterbysample);
	void SetDmaAlgo();
	int GetBufferAvailable();
	int GetUserMemIndex();
	int PushSample(int Index);
		
};
#endif
