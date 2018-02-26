extern "C"
{
#include "mailbox.h"
}
#include "gpio.h"
#include "raspberry_pi_revision.h"

gpio::gpio(uint32_t base, uint32_t len)
{
	
	gpioreg=( uint32_t *)mapmem(base,len);

}

int gpio::setmode(uint32_t gpio, uint32_t mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpioreg[reg] = (gpioreg[reg] & ~(7<<shift)) | (mode<<shift);

   return 0;
}

uint32_t gpio::GetPeripheralBase()
{
	RASPBERRY_PI_INFO_T info;
	uint32_t  BCM2708_PERI_BASE=0;
    if (getRaspberryPiInformation(&info) > 0)
	{
		if(info.peripheralBase==RPI_BROADCOM_2835_PERIPHERAL_BASE)
		{
			BCM2708_PERI_BASE = info.peripheralBase ;
		}

		if((info.peripheralBase==RPI_BROADCOM_2836_PERIPHERAL_BASE)||(info.peripheralBase==RPI_BROADCOM_2837_PERIPHERAL_BASE))
		{
			BCM2708_PERI_BASE = info.peripheralBase ;
		}
	}
	return BCM2708_PERI_BASE;
}


//******************** DMA Registers ***************************************

dmagpio::dmagpio():gpio(GetPeripheralBase()+DMA_BASE,DMA_LEN)
{
}
