

#include "RpiGpio.h"
#include "mailbox.h"
#include "raspberry_pi_revision.h"

static volatile unsigned int BCM2708_PERI_BASE;	
static uint32_t dram_phys_base;


char InitGpio()
{
	int   rev, mem, maker, overVolted ;
	//printf("*********** Init GPIO *************\n");
	RASPBERRY_PI_INFO_T info;
    
    	if (getRaspberryPiInformation(&info) > 0)
	{
		if(info.peripheralBase==RPI_BROADCOM_2835_PERIPHERAL_BASE)
		{
			BCM2708_PERI_BASE = info.peripheralBase ;
			 dram_phys_base   =  0x40000000;
			 mem_flag         =  0x0c;
		}

		if((info.peripheralBase==RPI_BROADCOM_2836_PERIPHERAL_BASE)||(info.peripheralBase==RPI_BROADCOM_2837_PERIPHERAL_BASE))
		{
			BCM2708_PERI_BASE = info.peripheralBase ;
			 dram_phys_base   =  0xc0000000;
			 mem_flag         =  0x04;
		}
	}

	DisplayInfo();
	
	dma_reg = map_peripheral(DMA_BASE, DMA_LEN);
	pwm_reg = map_peripheral(PWM_BASE, PWM_LEN);
	clk_reg = map_peripheral(CLK_BASE, CLK_LEN);

	pcm_reg =  map_peripheral(PCM_BASE, PCM_LEN); 
	gpio_reg = map_peripheral(GPIO_BASE, GPIO_LEN);
	pad_gpios_reg = map_peripheral(PADS_GPIO, PADS_GPIO_LEN);

	return 1;
}

void * map_peripheral(uint32_t base, uint32_t len)
{
	void * vaddr;
	vaddr=mapmem(base,len);
	//printf("Vaddr =%lx \n",vaddr);
	return vaddr;
}


int gpioSetMode(unsigned gpio, unsigned mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpio_reg[reg] = (gpio_reg[reg] & ~(7<<shift)) | (mode<<shift);

   return 0;
}

void DisplayInfo()
{
	RASPBERRY_PI_INFO_T info;
    
    	if (getRaspberryPiInformation(&info) > 0)
	{
	printf("memory: %s\n", raspberryPiMemoryToString(info.memory));

        printf("processor: %s\n",
               raspberryPiProcessorToString(info.processor));

        printf("i2cDevice: %s\n",
               raspberryPiI2CDeviceToString(info.i2cDevice));

        printf("model: %s\n",
               raspberryPiModelToString(info.model));

        printf("manufacturer: %s\n",
               raspberryPiManufacturerToString(info.manufacturer));

        printf("pcb revision: %d\n", info.pcbRevision);

        printf("warranty void: %s\n", (info.warrantyBit) ? "yes" : "no");

        printf("revision: %04x\n", info.revisionNumber);
        printf("peripheral base: 0x%x\n", info.peripheralBase);
	}
}


