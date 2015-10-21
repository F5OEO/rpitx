#ifndef RPI_GPIO
#define RPI_GPIO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>

char InitGpio();
void piBoardId (int *model, int *rev, int *mem, int *maker, int *overVolted);

int model;
 uint32_t mem_flag;

volatile uint32_t *pwm_reg;
 volatile uint32_t *clk_reg;
 volatile uint32_t *dma_reg;
volatile uint32_t *gpio_reg;
 volatile uint32_t *pcm_reg;
 volatile uint32_t *pad_gpios_reg;

void * map_peripheral(uint32_t base, uint32_t len);
int gpioSetMode(unsigned gpio, unsigned mode);


#define DMA_BASE		(BCM2708_PERI_BASE + 0x00007000 )
#define DMA_LEN			0xF00
#define PWM_BASE		(BCM2708_PERI_BASE + 0x0020C000)
#define PWM_LEN			0x28
#define CLK_BASE	        (BCM2708_PERI_BASE + 0x00101000)
#define CLK_LEN			0xA8
#define GPIO_BASE		(BCM2708_PERI_BASE + 0x00200000)
#define GPIO_LEN		0xB4
#define PCM_BASE		(BCM2708_PERI_BASE + 0x00203000)
#define PCM_LEN			0x24

#define PADS_GPIO (BCM2708_PERI_BASE + 0x00100000)
#define PADS_GPIO_LEN (0x40/4)
#define PADS_GPIO_0 (0x2C/4)
#define PADS_GPIO_1 (0x30/4)
#define PADS_GPIO_2 (0x34/4)


#define PWM_CTL			(0x00/4)
#define PWM_DMAC		(0x08/4)
#define PWM_RNG1		(0x10/4)
#define PWM_RNG2		(0x20/4)
#define PWM_FIFO		(0x18/4)

#define PWMCLK_CNTL		40
#define PWMCLK_DIV		41

#define GPCLK_CNTL		(0x70/4)
#define GPCLK_DIV		(0x74/4)


#define PWMCTL_MSEN2 (1<<15)
#define PWMCTL_USEF2 (1<<13)
#define PWMCTL_RPTL2 (1<<10)
#define PWMCTL_MODE2 (1<<9)
#define PWMCTL_PWEN2 (1<<8)

#define PWMCTL_MSEN1 (1<<7)
#define PWMCTL_CLRF (1<<6)
#define PWMCTL_USEF1 (1<<5)
#define PWMCTL_POLA1 (1<<4)
#define PWMCTL_RPTL1 (1<<2)
#define PWMCTL_MODE1 (1<<1)
#define PWMCTL_PWEN1 (1<<0)


#define PWMDMAC_ENAB		(1<<31)
#define PWMDMAC_THRSHLD		((15<<8)|(15<<0))

#define PCM_CS_A		(0x00/4)
#define PCM_FIFO_A		(0x04/4)
#define PCM_MODE_A		(0x08/4)
#define PCM_RXC_A		(0x0c/4)
#define PCM_TXC_A		(0x10/4)
#define PCM_DREQ_A		(0x14/4)
#define PCM_INTEN_A		(0x18/4)
#define PCM_INT_STC_A		(0x1c/4)
#define PCM_GRAY		(0x20/4)

#define PCMCLK_CNTL		38
#define PCMCLK_DIV		39

#define GPFSEL0			(0x00/4)
#define GPFSEL1    		(0x04/4)
#define GPFSEL2   		(0x08/4)
#endif
