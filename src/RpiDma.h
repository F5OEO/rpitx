#ifndef RPI_DMA
#define RPI_DMA


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mailbox.h"

char InitDma(void *FunctionTerminate, int* skipSignals);
uint32_t mem_virt_to_phys(volatile void *virt);
uint32_t mem_phys_to_virt(volatile uint32_t phys);

#define NBSAMPLES_PWM_FREQ_MAX 500
#define NUM_CB_PWM_FREQUENCY 8
//#define MBFILE			DEVICE_FILE_NAME	/* From mailbox.h */ 
#define NUM_SAMPLES_MAX		(4000)
#define CBS_SIZE_BY_SAMPLE	(3) 
#define NUM_CBS_MAIN 		((NUM_SAMPLES_MAX * CBS_SIZE_BY_SAMPLE))
#define NUM_CBS			(NUM_CBS_MAIN)

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

#define BUS_TO_PHYS(x) ((x)&~0xC0000000)

#define DMA_CS_RESET    (1<<31)
#define DMA_CS_ABORT    (1<<30)
#define DMA_CS_DISDEBUG (1<<28)
#define DMA_CS_END      (1<<1)
#define DMA_CS_ACTIVE   (1<<0)
#define DMA_CS_PRIORITY(x) ((x)&0xf << 16)
#define DMA_CS_PANIC_PRIORITY(x) ((x)&0xf << 20)

#define PAGE_SIZE	4096
#define PAGE_SHIFT	12
#define NUM_PAGES	((sizeof(struct control_data_s) + PAGE_SIZE - 1) >> PAGE_SHIFT)

struct {
	int handle;		/* From mbox_open() */
	unsigned mem_ref;	/* From mem_alloc() */
	unsigned bus_addr;	/* From mem_lock() */
	uint8_t *virt_addr;	/* From mapmem() */
} mbox;


// The GPU reserves channels 1, 3, 6, and 7 (kernel mask dma.dmachans=0x7f35)
// And, I think sdcard will always get 2 and fbturbo zero
//8 Crash
// 5 seems OK, 14 ALSO
//cat /sys/module/dma/parameters/dmachans
//On Wheezy
//32565: 111111100110101
//On Jessie Pi2:
//3893 :    111100110101
// USE CHANNEL 4 AND 5 which seems to be free
// On Jessie, channel 4 and 5 seems to crash : set to DMA 8 .
#define DMA_CHANNEL_WHEEZY 14
#define DMA_CHANNEL_JESSIE 5  
//#define DMA_CHANNEL_PWMFREQUENCY 5

char DMA_CHANNEL;

//#define DMA_CHANNEL 8

#define DMA_CHANNEL_PWMFREQUENCY (DMA_CHANNEL+1)

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

#define PWM_STEP_MAXI 200

typedef struct {
	
	uint32_t FrequencyTab[PWM_STEP_MAXI];
	uint32_t Amplitude1;
	uint32_t Amplitude2;
	
} sample_t;


struct control_data_s {
	dma_cb_t cb[NUM_CBS];//+1 ??
	
	sample_t sample[NUM_SAMPLES_MAX];
	
	
	
	
};

struct control_data_s *ctl;

#endif
