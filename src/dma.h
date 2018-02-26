#ifndef DEF_DMA
#define DEF_DMA
#include "stdint.h"
#include "gpio.h"

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
    } dma_cb_t;

    typedef struct {
	uint8_t *virtaddr;
	uint32_t physaddr;
    } page_map_t;

    page_map_t *page_map;

    uint8_t *virtbase;
    int NumPages=0;
	int channel; //DMA Channel
	dmagpio dma_reg;
    public:
    dma_cb_t *cbarray;
    unsigned int *usermem;



    dma(int Channel,int CBSize,int UserMemSize,unsigned int mem_flag);
    ~dma();
    uint32_t mem_virt_to_phys(volatile void *virt);
    uint32_t mem_phys_to_virt(volatile uint32_t phys);

    int start();
    int stop();
    
};
#endif
