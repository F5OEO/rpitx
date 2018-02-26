#ifndef DEF_GPIO
#define DEF_GPIO
#include "stdint.h"

class gpio
{
    
    public:
    volatile uint32_t *gpioreg;
    gpio(uint32_t base, uint32_t len);
    int setmode(uint32_t gpio, uint32_t mode);
    uint32_t GetPeripheralBase();
};


#define DMA_BASE		(0x00007000 )
#define DMA_LEN			0xF00

#define BCM2708_DMA_SRC_IGNOR           (1<<11)
#define BCM2708_DMA_SRC_INC		(1<<8)
#define BCM2708_DMA_DST_IGNOR           (1<<7)
#define BCM2708_DMA_NO_WIDE_BURSTS	(1<<26)
#define BCM2708_DMA_WAIT_RESP		(1<<3)

#define BCM2708_DMA_D_DREQ		(1<<6)
#define BCM2708_DMA_PER_MAP(x)		((x)<<16)
#define BCM2708_DMA_END			(1<<1)
#define BCM2708_DMA_RESET		(1<<31)
#define BCM2708_DMA_ABORT		(1<<30)
#define BCM2708_DMA_INT			(1<<2)

#define DMA_CS			(0x00/4)
#define DMA_CONBLK_AD		(0x04/4)
#define DMA_DEBUG		(0x20/4)

#define DMA_CS_RESET    (1<<31)
#define DMA_CS_ABORT    (1<<30)
#define DMA_CS_DISDEBUG (1<<28)
#define DMA_CS_END      (1<<1)
#define DMA_CS_ACTIVE   (1<<0)
#define DMA_CS_PRIORITY(x) ((x)&0xf << 16)
#define DMA_CS_PANIC_PRIORITY(x) ((x)&0xf << 20)

class dmagpio:public gpio
{
    
    public:
    dmagpio();
	
        
};


#endif
