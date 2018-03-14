extern "C"
{
#include "mailbox.h"
}
#include "gpio.h"
#include "raspberry_pi_revision.h"
#include "stdio.h"
#include <unistd.h>

gpio::gpio(uint32_t base, uint32_t len)
{
	
	gpioreg=( uint32_t *)mapmem(base,len);

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

// ***************** CLK Registers *****************************************
clkgpio::clkgpio():gpio(GetPeripheralBase()+CLK_BASE,CLK_LEN)
{
	
}

clkgpio::~clkgpio()
{
	gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(0 << 4)  ; //4 is START CLK
	
	usleep(100);
}

int clkgpio::SetPllNumber(int PllNo,int MashType)
{
	//print_clock_tree();
	if(PllNo<8)
		pllnumber=PllNo;
	else
		pllnumber=clk_pllc;
	
	if(MashType<4)
		Mash=MashType;
	else
		Mash=0;
	gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber/*|(1 << 5)*/  ; //5 is Reset CLK
	usleep(100);
	Pllfrequency=GetPllFrequency(pllnumber);
	return 0;
}

uint64_t clkgpio::GetPllFrequency(int PllNo)
{
	uint64_t Freq=0;
	switch(PllNo)
	{
		case clk_osc:Freq=XOSC_FREQUENCY;break;
		case clk_plla:Freq=XOSC_FREQUENCY*((uint64_t)gpioreg[PLLA_CTRL]&0x3ff) +XOSC_FREQUENCY*(uint64_t)gpioreg[PLLA_FRAC]/(1<<20);break;
		//case clk_pllb:Freq=XOSC_FREQUENCY*((uint64_t)gpioreg[PLLB_CTRL]&0x3ff) +XOSC_FREQUENCY*(uint64_t)gpioreg[PLLB_FRAC]/(1<<20);break;
		case clk_pllc:Freq=XOSC_FREQUENCY*((uint64_t)gpioreg[PLLC_CTRL]&0x3ff) +XOSC_FREQUENCY*(uint64_t)gpioreg[PLLC_FRAC]/(1<<20);break;
		case clk_plld:Freq=(XOSC_FREQUENCY*((uint64_t)gpioreg[PLLD_CTRL]&0x3ff) +(XOSC_FREQUENCY*(uint64_t)gpioreg[PLLD_FRAC])/(1<<20))/(gpioreg[PLLD_PER]>>1);break;
		case clk_hdmi:Freq=XOSC_FREQUENCY*((uint64_t)gpioreg[PLLH_CTRL]&0x3ff) +XOSC_FREQUENCY*(uint64_t)gpioreg[PLLH_FRAC]/(1<<20);break;
	}
	fprintf(stderr,"Freq = %lld\n",Freq);

	return Freq;
}


int clkgpio::SetClkDivFrac(uint32_t Div,uint32_t Frac)
{
	
	gpioreg[GPCLK_DIV] = 0x5A000000 | ((Div)<<12) | Frac;
	usleep(100);
	fprintf(stderr,"Clk Number %d div %d frac %d\n",pllnumber,Div,Frac);
	//gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber |(1<<4)  ; //4 is START CLK
	//	usleep(10);
	return 0;

}

int clkgpio::SetMasterMultFrac(uint32_t Mult,uint32_t Frac)
{
	
	fprintf(stderr,"Master Mult %d Frac %d\n",Mult,Frac);
	gpioreg[PLLA_CTRL] = (0x5a<<24) | (0x21<<12) | Mult;
	usleep(100);
	gpioreg[PLLA_FRAC]= 0x5A000000 | Frac  ; 
	
	return 0;

}

int clkgpio::SetFrequency(int Frequency)
{
	if(ModulateFromMasterPLL)
	{
		double FloatMult=((double)(CentralFrequency+Frequency)*PllFixDivider)/(double)(XOSC_FREQUENCY);
		uint32_t freqctl = FloatMult*((double)(1<<20)) ; 
		int IntMultiply= freqctl>>20; // Need to be calculated to have a center frequency
		freqctl&=0xFFFFF; // Fractionnal is 20bits
		uint32_t FracMultiply=freqctl&0xFFFFF; 
		//gpioreg[PLLA_FRAC]= 0x5A000000 | FracMultiply  ; // Only Frac is Sent
		SetMasterMultFrac(IntMultiply,FracMultiply);
				
	}
	else
	{
		double Freqresult=(double)Pllfrequency/(double)(CentralFrequency+Frequency);
		uint32_t FreqDivider=(uint32_t)Freqresult;
		uint32_t FreqFractionnal=(uint32_t) (4096*(Freqresult-(double)FreqDivider));
		if((FreqDivider>4096)||(FreqDivider<2)) fprintf(stderr,"Frequency out of range\n");
		printf("DIV/FRAC %u/%u \n",FreqDivider,FreqFractionnal);
	
		SetClkDivFrac(FreqDivider,FreqFractionnal);
	}
	
	return 0;

}

uint32_t clkgpio::GetMasterFrac(int Frequency)
{
	if(ModulateFromMasterPLL)
	{
		double FloatMult=((double)(CentralFrequency+Frequency)*PllFixDivider)/(double)(XOSC_FREQUENCY);
		uint32_t freqctl = FloatMult*((double)(1<<20)) ; 
		int IntMultiply= freqctl>>20; // Need to be calculated to have a center frequency
		freqctl&=0xFFFFF; // Fractionnal is 20bits
		uint32_t FracMultiply=freqctl&0xFFFFF; 
		return FracMultiply;
	}
	else
		return 0; //Not in Master CLk mode
	
}

int clkgpio::ComputeBestLO(uint64_t Frequency,int Bandwidth)
{ 
	// Algorithm adapted from https://github.com/SaucySoliton/PiFmRds/blob/master/src/pi_fm_rds.c
	// Choose an integer divider for GPCLK0
    // 
    // There may be improvements possible to this algorithm. 
    double xtal_freq_recip=1.0/19.2e6; // todo PPM correction 
    int best_divider=0;

    
      int solution_count=0;
      //printf("carrier:%3.2f ",carrier_freq/1e6);
      int divider,min_int_multiplier,max_int_multiplier, fom, int_multiplier, best_fom=0;
      double frac_multiplier;
      best_divider=0;
      for( divider=1;divider<4096;divider++)
      {
        if( Frequency*divider <  600e6 ) continue; // widest accepted frequency range
        if( Frequency*divider > 1800e6 ) break;

        max_int_multiplier=((int)((double)(Frequency+Bandwidth)*divider*xtal_freq_recip));
        min_int_multiplier=((int)((double)(Frequency-Bandwidth)*divider*xtal_freq_recip));
        if( min_int_multiplier!=max_int_multiplier ) continue; // don't cross integer boundary

        solution_count++;  // if we make it here the solution is acceptable, 
        fom=0;             // but we want a good solution

        if( Frequency*divider >  900e6 ) fom++; // prefer freqs closer to 1000
        if( Frequency*divider < 1100e6 ) fom++;
        if( Frequency*divider >  800e6 ) fom++; // accepted frequency range
        if( Frequency*divider < 1200e6 ) fom++;
        

        frac_multiplier=((double)(Frequency)*divider*xtal_freq_recip);
        int_multiplier = (int) frac_multiplier;
        frac_multiplier = frac_multiplier - int_multiplier;
		if((int_multiplier%2)==0) fom++;
        if( (frac_multiplier>0.4) && (frac_multiplier<0.6) ) fom+=2; // prefer mulipliers away from integer boundaries 


        //if( divider%2 == 1 ) fom+=2; // prefer odd dividers
        // Even and odd dividers could have different harmonic content,
        // but the latest measurements have shown no significant difference.


        //printf(" multiplier:%f divider:%d VCO: %4.1fMHz\n",carrier_freq*divider*xtal_freq_recip,divider,(double)carrier_freq*divider/1e6);
        if( fom > best_fom )
        {
            best_fom=fom;
            best_divider=divider;
        }
      }
		if(solution_count>0)
		{
			  PllFixDivider=best_divider;
      		  fprintf(stderr," multiplier:%f divider:%d VCO: %4.1fMHz\n",Frequency*best_divider*xtal_freq_recip,best_divider,(double)Frequency*best_divider/1e6);
			  return 0;
		}
		else
		{
			fprintf(stderr,"Central frequency not available !!!!!!\n");
			return -1;		
    	}
}

int clkgpio::SetCenterFrequency(uint64_t Frequency,int Bandwidth)
{
	CentralFrequency=Frequency;
	if(ModulateFromMasterPLL)
	{
		//Choose best PLLDiv and Div
		ComputeBestLO(Frequency,Bandwidth); //FixeDivider update
	
		SetFrequency(0);
		usleep(1000);
		if((gpioreg[CM_LOCK]&CM_LOCK_FLOCKA)>0) 
			fprintf(stderr,"Master PLLA Locked\n");
		else
			fprintf(stderr,"Warning ! Master PLLA NOT Locked !!!!\n");
		SetClkDivFrac(PllFixDivider,0); // NO MASH !!!!
		usleep(100);
		
		usleep(100);
		gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(1 << 4)  ; //4 is START CLK
		usleep(100);
		gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(1 << 4)  ; //4 is START CLK
		usleep(100);
	}
	else
	{
		GetPllFrequency(pllnumber);// Be sure to get the master PLL frequency
		gpioreg[GPCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(1 << 4)  ; //4 is START CLK
	}
	return 0;	
}

void clkgpio::SetPhase(bool inversed)
{
	uint32_t StateBefore=clkgpio::gpioreg[GPCLK_CNTL];
	clkgpio::gpioreg[GPCLK_CNTL]= (0x5A<<24) | StateBefore | ((inversed?1:0)<<8) | 1<<5;
	clkgpio::gpioreg[GPCLK_CNTL]= (0x5A<<24) | StateBefore | ((inversed?1:0)<<8) | 0<<5;
}

void clkgpio::SetAdvancedPllMode(bool Advanced)
{
	ModulateFromMasterPLL=Advanced;
	if(ModulateFromMasterPLL)
	{
		SetPllNumber(clk_plla,0); // Use PPL_A , Do not USE MASH which generates spurious
		gpioreg[0x104/4]=0x5A00020A; // Enable Plla_PER
		usleep(100);
		
		uint32_t ana[4];
		for(int i=3;i>=0;i--)
		{
			ana[i]=gpioreg[(0x1010/4)+i];
		}
		
		//ana[1]&=~(1<<14); // No use prediv means Frequency
		ana[1]|=(1<<14); // use prediv means Frequency*2
		for(int i=3;i>=0;i--)
		{
			gpioreg[(0x1010/4)+i]=(0x5A<<24)|ana[i];
		}
	
		
		usleep(100);
		gpioreg[PLLA_PER]=0x5A000002; // Div ? 
		usleep(100);
	}
}

void clkgpio::print_clock_tree(void)
{

   printf("PLLC_DIG0=%08x\n",gpioreg[(0x1020/4)]);
   printf("PLLC_DIG1=%08x\n",gpioreg[(0x1024/4)]);
   printf("PLLC_DIG2=%08x\n",gpioreg[(0x1028/4)]);
   printf("PLLC_DIG3=%08x\n",gpioreg[(0x102c/4)]);
   printf("PLLC_ANA0=%08x\n",gpioreg[(0x1030/4)]);
   printf("PLLC_ANA1=%08x\n",gpioreg[(0x1034/4)]);
   printf("PLLC_ANA2=%08x\n",gpioreg[(0x1038/4)]);
   printf("PLLC_ANA3=%08x\n",gpioreg[(0x103c/4)]);
   printf("PLLC_DIG0R=%08x\n",gpioreg[(0x1820/4)]);
   printf("PLLC_DIG1R=%08x\n",gpioreg[(0x1824/4)]);
   printf("PLLC_DIG2R=%08x\n",gpioreg[(0x1828/4)]);
   printf("PLLC_DIG3R=%08x\n",gpioreg[(0x182c/4)]);

   printf("PLLA_ANA0=%08x\n",gpioreg[(0x1010/4)]);	
   printf("PLLA_ANA1=%08x prediv=%d\n",gpioreg[(0x1014/4)],(gpioreg[(0x1014/4)]>>14)&1);	
   printf("PLLA_ANA2=%08x\n",gpioreg[(0x1018/4)]);	
   printf("PLLA_ANA3=%08x\n",gpioreg[(0x101c/4)]);		
		
   printf("GNRIC CTL=%08x DIV=%8x  ",gpioreg[ 0],gpioreg[ 1]);
   printf("VPU   CTL=%08x DIV=%8x\n",gpioreg[ 2],gpioreg[ 3]);
   printf("SYS   CTL=%08x DIV=%8x  ",gpioreg[ 4],gpioreg[ 5]);
   printf("PERIA CTL=%08x DIV=%8x\n",gpioreg[ 6],gpioreg[ 7]);
   printf("PERII CTL=%08x DIV=%8x  ",gpioreg[ 8],gpioreg[ 9]);
   printf("H264  CTL=%08x DIV=%8x\n",gpioreg[10],gpioreg[11]);
   printf("ISP   CTL=%08x DIV=%8x  ",gpioreg[12],gpioreg[13]);
   printf("V3D   CTL=%08x DIV=%8x\n",gpioreg[14],gpioreg[15]);

   printf("CAM0  CTL=%08x DIV=%8x  ",gpioreg[16],gpioreg[17]);
   printf("CAM1  CTL=%08x DIV=%8x\n",gpioreg[18],gpioreg[19]);
   printf("CCP2  CTL=%08x DIV=%8x  ",gpioreg[20],gpioreg[21]);
   printf("DSI0E CTL=%08x DIV=%8x\n",gpioreg[22],gpioreg[23]);
   printf("DSI0P CTL=%08x DIV=%8x  ",gpioreg[24],gpioreg[25]);
   printf("DPI   CTL=%08x DIV=%8x\n",gpioreg[26],gpioreg[27]);
   printf("GP0   CTL=%08x DIV=%8x  ",gpioreg[0x70/4],gpioreg[0x74/4]);
   printf("GP1   CTL=%08x DIV=%8x\n",gpioreg[30],gpioreg[31]);

   printf("GP2   CTL=%08x DIV=%8x  ",gpioreg[32],gpioreg[33]);
   printf("HSM   CTL=%08x DIV=%8x\n",gpioreg[34],gpioreg[35]);
   printf("OTP   CTL=%08x DIV=%8x  ",gpioreg[36],gpioreg[37]);
   printf("PCM   CTL=%08x DIV=%8x\n",gpioreg[38],gpioreg[39]);
   printf("PWM   CTL=%08x DIV=%8x  ",gpioreg[40],gpioreg[41]);
   printf("SLIM  CTL=%08x DIV=%8x\n",gpioreg[42],gpioreg[43]);
   printf("SMI   CTL=%08x DIV=%8x  ",gpioreg[44],gpioreg[45]);
   printf("SMPS  CTL=%08x DIV=%8x\n",gpioreg[46],gpioreg[47]);

   printf("TCNT  CTL=%08x DIV=%8x  ",gpioreg[48],gpioreg[49]);
   printf("TEC   CTL=%08x DIV=%8x\n",gpioreg[50],gpioreg[51]);
   printf("TD0   CTL=%08x DIV=%8x  ",gpioreg[52],gpioreg[53]);
   printf("TD1   CTL=%08x DIV=%8x\n",gpioreg[54],gpioreg[55]);

   printf("TSENS CTL=%08x DIV=%8x  ",gpioreg[56],gpioreg[57]);
   printf("TIMER CTL=%08x DIV=%8x\n",gpioreg[58],gpioreg[59]);
   printf("UART  CTL=%08x DIV=%8x  ",gpioreg[60],gpioreg[61]);
   printf("VEC   CTL=%08x DIV=%8x\n",gpioreg[62],gpioreg[63]);
   	

   printf("PULSE CTL=%08x DIV=%8x  ",gpioreg[100],gpioreg[101]);
   printf("PLLT  CTL=%08x DIV=????????\n",gpioreg[76]);

   printf("DSI1E CTL=%08x DIV=%8x  ",gpioreg[86],gpioreg[87]);
   printf("DSI1P CTL=%08x DIV=%8x\n",gpioreg[88],gpioreg[89]);
   printf("AVE0  CTL=%08x DIV=%8x\n",gpioreg[90],gpioreg[91]);

   printf("CMPLLA=%08x  ",gpioreg[0x104/4]);
   printf("CMPLLC=%08x \n",gpioreg[0x108/4]);
   printf("CMPLLD=%08x   ",gpioreg[0x10C/4]);
   printf("CMPLLH=%08x \n",gpioreg[0x110/4]);
	
     printf("EMMC  CTL=%08x DIV=%8x\n",gpioreg[112],gpioreg[113]);
	   printf("EMMC  CTL=%08x DIV=%8x\n",gpioreg[112],gpioreg[113]);
   printf("EMMC  CTL=%08x DIV=%8x\n",gpioreg[112],gpioreg[113]);	
	

   // Sometimes calculated frequencies are off by a factor of 2
   // ANA1 bit 14 may indicate that a /2 prescaler is active
   printf("PLLA PDIV=%d NDIV=%d FRAC=%d  ",(gpioreg[PLLA_CTRL]>>16) ,gpioreg[PLLA_CTRL]&0x3ff, gpioreg[PLLA_FRAC] );
   printf(" %f MHz\n",19.2* ((float)(gpioreg[PLLA_CTRL]&0x3ff) + ((float)gpioreg[PLLA_FRAC])/((float)(1<<20))) );
   printf("DSI0=%d CORE=%d PER=%d CCP2=%d\n\n",gpioreg[PLLA_DSI0],gpioreg[PLLA_CORE],gpioreg[PLLA_PER],gpioreg[PLLA_CCP2]);


   printf("PLLB PDIV=%d NDIV=%d FRAC=%d  ",(gpioreg[PLLB_CTRL]>>16) ,gpioreg[PLLB_CTRL]&0x3ff, gpioreg[PLLB_FRAC] );
   printf(" %f MHz\n",19.2* ((float)(gpioreg[PLLB_CTRL]&0x3ff) + ((float)gpioreg[PLLB_FRAC])/((float)(1<<20))) );
   printf("ARM=%d SP0=%d SP1=%d SP2=%d\n\n",gpioreg[PLLB_ARM],gpioreg[PLLB_SP0],gpioreg[PLLB_SP1],gpioreg[PLLB_SP2]);

   printf("PLLC PDIV=%d NDIV=%d FRAC=%d  ",(gpioreg[PLLC_CTRL]>>16) ,gpioreg[PLLC_CTRL]&0x3ff, gpioreg[PLLC_FRAC] );
   printf(" %f MHz\n",19.2* ((float)(gpioreg[PLLC_CTRL]&0x3ff) + ((float)gpioreg[PLLC_FRAC])/((float)(1<<20))) );
   printf("CORE2=%d CORE1=%d PER=%d CORE0=%d\n\n",gpioreg[PLLC_CORE2],gpioreg[PLLC_CORE1],gpioreg[PLLC_PER],gpioreg[PLLC_CORE0]);

   printf("PLLD %x PDIV=%d NDIV=%d FRAC=%d  ",gpioreg[PLLD_CTRL],(gpioreg[PLLD_CTRL]>>16) ,gpioreg[PLLD_CTRL]&0x3ff, gpioreg[PLLD_FRAC] );
   printf(" %f MHz\n",19.2* ((float)(gpioreg[PLLD_CTRL]&0x3ff) + ((float)gpioreg[PLLD_FRAC])/((float)(1<<20))) );
   printf("DSI0=%d CORE=%d PER=%d DSI1=%d\n\n",gpioreg[PLLD_DSI0],gpioreg[PLLD_CORE],gpioreg[PLLD_PER],gpioreg[PLLD_DSI1]);

   printf("PLLH PDIV=%d NDIV=%d FRAC=%d  ",(gpioreg[PLLH_CTRL]>>16) ,gpioreg[PLLH_CTRL]&0x3ff, gpioreg[PLLH_FRAC] );
   printf(" %f MHz\n",19.2* ((float)(gpioreg[PLLH_CTRL]&0x3ff) + ((float)gpioreg[PLLH_FRAC])/((float)(1<<20))) );
   printf("AUX=%d RCAL=%d PIX=%d STS=%d\n\n",gpioreg[PLLH_AUX],gpioreg[PLLH_RCAL],gpioreg[PLLH_PIX],gpioreg[PLLH_STS]);


}

void clkgpio::enableclk(int gpio)
{
	switch(gpio)
	{
		case 4:	gengpio.setmode(gpio,fsel_alt0);break;
	    case 20:gengpio.setmode(gpio,fsel_alt5);break;	
		case 32:gengpio.setmode(gpio,fsel_alt0);break;
		case 34:gengpio.setmode(gpio,fsel_alt0);break;
		default: fprintf(stderr,"gpio %d has no clk - available(4,20,32,34)\n",gpio);break; 
	}
	usleep(100);
}

void clkgpio::disableclk(int gpio)
{
	gengpio.setmode(gpio,fsel_input);
	
}

// ************************************** GENERAL GPIO *****************************************************

generalgpio::generalgpio():gpio(GetPeripheralBase()+GENERAL_BASE,GENERAL_LEN)
{
}

generalgpio::~generalgpio()
{
	
}

int generalgpio::setmode(uint32_t gpio, uint32_t mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpioreg[reg] = (gpioreg[reg] & ~(7<<shift)) | (mode<<shift);

   return 0;
}


// ********************************** PWM GPIO **********************************

pwmgpio::pwmgpio():gpio(GetPeripheralBase()+PWM_BASE,PWM_LEN)
{
	
	gpioreg[PWM_CTL] = 0;
}

pwmgpio::~pwmgpio()
{
	
	gpioreg[PWM_CTL] = 0;
	gpioreg[PWM_DMAC] = 0;
}

void pwmgpio::enablepwm(int gpio,int PwmNumber)
{
	if(PwmNumber==0)
	{
		switch(gpio)
		{
			case 12:gengpio.setmode(gpio,fsel_alt0);break;
			case 18:gengpio.setmode(gpio,fsel_alt5);break;	
			case 40:gengpio.setmode(gpio,fsel_alt0);break;
			
			default: fprintf(stderr,"gpio %d has no pwm - available(12,18,40)\n",gpio);break; 
		}
	}
	if(PwmNumber==1)
	{
		switch(gpio)
		{
			case 13:gengpio.setmode(gpio,fsel_alt0);break;
			case 19:gengpio.setmode(gpio,fsel_alt5);break;	
			case 41:gengpio.setmode(gpio,fsel_alt0);break;
			case 45:gengpio.setmode(gpio,fsel_alt0);break;
			default: fprintf(stderr,"gpio %d has no pwm - available(13,19,41,45)\n",gpio);break; 
		}
	}
	usleep(100);
}

void pwmgpio::disablepwm(int gpio)
{
	gengpio.setmode(gpio,fsel_input);
	
}

int pwmgpio::SetPllNumber(int PllNo,int MashType)
{
	if(PllNo<8)
		pllnumber=PllNo;
	else
		pllnumber=clk_pllc;	
	if(MashType<4)
		Mash=MashType;
	else
		Mash=0;
	clk.gpioreg[PWMCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(0 << 4)  ; //4 is STOP CLK
	usleep(100);
	Pllfrequency=GetPllFrequency(pllnumber);
	return 0;
}

uint64_t pwmgpio::GetPllFrequency(int PllNo)
{
	return clk.GetPllFrequency(PllNo);

}

int pwmgpio::SetFrequency(uint64_t Frequency)
{
	Prediv=32; // Fixe for now , need investigation if not 32 !!!! FixMe !
	double Freqresult=(double)Pllfrequency/(double)(Frequency*Prediv);
	uint32_t FreqDivider=(uint32_t)Freqresult;
	uint32_t FreqFractionnal=(uint32_t) (4096*(Freqresult-(double)FreqDivider));
	if((FreqDivider>4096)||(FreqDivider<2)) fprintf(stderr,"Frequency out of range\n");
	fprintf(stderr,"PWM clk=%d / %d\n",FreqDivider,FreqFractionnal);
	clk.gpioreg[PWMCLK_DIV] = 0x5A000000 | ((FreqDivider)<<12) | FreqFractionnal;
	
	usleep(100);
	clk.gpioreg[PWMCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(1 << 4)  ; //4 is STAR CLK
	usleep(100);
	
	
	SetPrediv(Prediv);	//SetMode should be called before
	return 0;

}

void pwmgpio::SetMode(int Mode)
{
	if((Mode>=pwm1pin)&&(Mode<=pwm1pinrepeat))
		ModePwm=Mode;
}

int pwmgpio::SetPrediv(int predivisor) //Mode should be only for SYNC or a Data serializer : Todo
{
		Prediv=predivisor;
		if(Prediv>32) 
		{
			fprintf(stderr,"PWM Prediv is max 32\n");
			Prediv=2;
		}
		fprintf(stderr,"PWM Prediv %d\n",Prediv);
		gpioreg[PWM_RNG1] = Prediv;// 250 -> 8KHZ
		usleep(100);
		gpioreg[PWM_RNG2] = Prediv;// 32 Mandatory for Serial Mode without gap
	
		//gpioreg[PWM_FIFO]=0xAAAAAAAA;

		gpioreg[PWM_DMAC] = PWMDMAC_ENAB | PWMDMAC_THRSHLD;
		usleep(100);
		gpioreg[PWM_CTL] = PWMCTL_CLRF;
		usleep(100);
		
		//gpioreg[PWM_CTL] = PWMCTL_USEF1| PWMCTL_MODE1| PWMCTL_PWEN1|PWMCTL_MSEN1;
		switch(ModePwm)
		{
			case pwm1pin:gpioreg[PWM_CTL] = PWMCTL_USEF1| PWMCTL_MODE1| PWMCTL_PWEN1|PWMCTL_MSEN1;break; // All serial go to 1 pin
			case pwm2pin:gpioreg[PWM_CTL] = PWMCTL_USEF2|PWMCTL_PWEN2|PWMCTL_MODE2|PWMCTL_USEF1| PWMCTL_MODE1| PWMCTL_PWEN1;break;// Alternate bit to pin 1 and 2
			case pwm1pinrepeat:gpioreg[PWM_CTL] = PWMCTL_USEF1| PWMCTL_MODE1| PWMCTL_PWEN1|PWMCTL_RPTL1;break; // All serial go to 1 pin, repeat if empty : RF mode with PWM
		}
		usleep(100);
	
	return 0;

}
		
// ********************************** PCM GPIO (I2S) **********************************

pcmgpio::pcmgpio():gpio(GetPeripheralBase()+PCM_BASE,PCM_LEN)
{
	gpioreg[PCM_CS_A] = 1;				// Disable Rx+Tx, Enable PCM block
}

pcmgpio::~pcmgpio()
{
	
}

int pcmgpio::SetPllNumber(int PllNo,int MashType)
{
	if(PllNo<8)
		pllnumber=PllNo;
	else
		pllnumber=clk_pllc;	
	if(MashType<4)
		Mash=MashType;
	else
		Mash=0;
	clk.gpioreg[PCMCLK_CNTL]= 0x5A000000 | (Mash << 9) | pllnumber|(1 << 4)  ; //4 is START CLK
	Pllfrequency=GetPllFrequency(pllnumber);
	return 0;
}

uint64_t pcmgpio::GetPllFrequency(int PllNo)
{
	return clk.GetPllFrequency(PllNo);

}

int pcmgpio::ComputePrediv(uint64_t Frequency)
{
	int prediv=5;
	for(prediv=10;prediv<1000;prediv++)
	{
		double Freqresult=(double)Pllfrequency/(double)(Frequency*prediv);
		if((Freqresult<4096.0)&&(Freqresult>2.0))
		{
			fprintf(stderr,"PCM prediv = %d\n",prediv);
			break;
		}
	}	
	return prediv;	
}

int pcmgpio::SetFrequency(uint64_t Frequency)
{
	Prediv=ComputePrediv(Frequency);
	double Freqresult=(double)Pllfrequency/(double)(Frequency*Prediv);
	uint32_t FreqDivider=(uint32_t)Freqresult;
	uint32_t FreqFractionnal=(uint32_t) (4096*(Freqresult-(double)FreqDivider));
	fprintf(stderr,"PCM clk=%d / %d\n",FreqDivider,FreqFractionnal);
	if((FreqDivider>4096)||(FreqDivider<2)) fprintf(stderr,"PCM Frequency out of range\n");
	clk.gpioreg[PCMCLK_DIV] = 0x5A000000 | ((FreqDivider)<<12) | FreqFractionnal;
	SetPrediv(Prediv);
	return 0;

}

int pcmgpio::SetPrediv(int predivisor) //Carefull we use a 10 fixe divisor for now : frequency is thus f/10 as a samplerate
{
	if(predivisor>1000)
	{
		fprintf(stderr,"PCM prediv should be <1000");
		predivisor=1000;
	}
	
	gpioreg[PCM_TXC_A] = 0<<31 | 1<<30 | 0<<20 | 0<<16; // 1 channel, 8 bits
	usleep(100);
	
	//printf("Nb PCM STEP (<1000):%d\n",NbStepPCM);
	gpioreg[PCM_MODE_A] = (predivisor-1)<<10; // SHOULD NOT EXCEED 1000 !!! 
	usleep(100);
	gpioreg[PCM_CS_A] |= 1<<4 | 1<<3;		// Clear FIFOs
	usleep(100);
	gpioreg[PCM_DREQ_A] = 64<<24 | 64<<8 ;		//TX Fifo PCM=64 DMA Req when one slot is free?
	usleep(100);
	gpioreg[PCM_CS_A] |= 1<<9;			// Enable DMA
	usleep(100);
	gpioreg[PCM_CS_A] |= 1<<2; //START TX PCM
	
	return 0;

}


// ********************************** PADGPIO (Amplitude) **********************************

padgpio::padgpio():gpio(GetPeripheralBase()+PADS_GPIO,PADS_GPIO_LEN)
{
	
}

padgpio::~padgpio()
{
	
}



