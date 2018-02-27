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


// Add for PLL frequency CTRL wihout divider
// https://github.com/raspberrypi/linux/blob/rpi-4.9.y/drivers/clk/bcm/clk-bcm2835.c
// See interesting patch for jitter https://github.com/raspberrypi/linux/commit/76527b4e6a5dbe55e0b2d8ab533c2388b36c86be

#define CLK_BASE	        (0x00101000)
#define CLK_LEN			0x1300

#define CORECLK_CNTL      (0x08/4)
#define CORECLK_DIV       (0x0c/4)
#define GPCLK_CNTL        (0x70/4)
#define GPCLK_DIV         (0x74/4)
#define EMMCCLK_CNTL     (0x1C0/4)
#define EMMCCLK_DIV      (0x1C4/4)


#define PLLA_CTRL (0x1100/4)
#define PLLA_FRAC (0x1200/4)
#define PLLA_DSI0 (0x1300/4)
#define PLLA_CORE (0x1400/4)
#define PLLA_PER  (0x1500/4)
#define PLLA_CCP2 (0x1600/4)

#define PLLB_CTRL  (0x11e0/4)
#define PLLB_FRAC  (0x12e0/4)
#define PLLB_ARM   (0x13e0/4)
#define PLLB_SP0   (0x14e0/4)
#define PLLB_SP1   (0x15e0/4)
#define PLLB_SP2   (0x16e0/4)

#define PLLC_CTRL  (0x1120/4)
#define PLLC_FRAC  (0x1220/4)
#define PLLC_CORE2 (0x1320/4)
#define PLLC_CORE1 (0x1420/4)
#define PLLC_PER   (0x1520/4)
#define PLLC_CORE0 (0x1620/4)

#define PLLD_CTRL (0x1140/4)
#define PLLD_FRAC (0x1240/4)
#define PLLD_DSI0 (0x1340/4)
#define PLLD_CORE (0x1440/4)
#define PLLD_PER  (0x1540/4)
#define PLLD_DSI1 (0x1640/4)

#define PLLH_CTRL (0x1160/4)
#define PLLH_FRAC (0x1260/4)
#define PLLH_AUX  (0x1360/4)
#define PLLH_RCAL (0x1460/4)
#define PLLH_PIX  (0x1560/4)
#define PLLH_STS  (0x1660/4)

#define XOSC_CTRL (0x1190/4)
#define XOSC_FREQUENCY 19200000

enum {clk_gnd,clk_osc,clk_debug0,clk_debug1,clk_plla,clk_pllc,clk_plld,clk_hdmi};

class clkgpio:public gpio
{
    int pllnumber;
	int Mash;
	uint64_t Pllfrequency;
	
    public:
    clkgpio();
	~clkgpio();
	int SetPllNumber(int PllNo,int MashType);
	uint64_t GetPllFrequency(int PllNo);
	void print_clock_tree(void);
	int SetFrequency(uint64_t Frequency); 
	
        
};


//************************************ GENERAL GPIO ***************************************

#define GENERAL_BASE		(0x00200000)
#define GENERAL_LEN		0xB4

#define GPFSEL0			(0x00/4)
#define GPFSEL1    		(0x04/4)
#define GPFSEL2   		(0x08/4)
#define GPPUD           (0x94/4)
#define GPPUDCLK0       (0x9C/4)

class generalgpio:public gpio
{
    
    public:
    generalgpio();
	~generalgpio();
    void enableclk();  
	void disableclk();
};

//************************************ PWM GPIO ***************************************

#define PWM_BASE		(0x0020C000)
#define PWM_LEN			0x28

#define PWM_CTL			(0x00/4)
#define PWM_DMAC		(0x08/4)
#define PWM_RNG1		(0x10/4)
#define PWM_RNG2		(0x20/4)
#define PWM_FIFO		(0x18/4)

#define PWMCLK_CNTL		(0x100/4) // Clk register
#define PWMCLK_DIV		(0x104/4) // Clk register


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

class pwmgpio:public gpio
{
    protected:
	clkgpio clk;
	int pllnumber;
	int Mash;
	uint64_t Pllfrequency;
    public:
    pwmgpio();
	~pwmgpio();
	int SetPllNumber(int PllNo,int MashType);
	uint64_t GetPllFrequency(int PllNo);
	int SetFrequency(uint64_t Frequency);
   int SetMode(int Mode);
};


#endif
