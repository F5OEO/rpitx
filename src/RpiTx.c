/*
 
   <rpitx is a software which use the GPIO of Raspberry Pi to transmit HF>

    Copyright (C) 2015  Evariste COURJAUD F5OEO (evaristec@gmail.com)

    Transmitting on HF band is surely not permitted without license (Hamradio for example).
    Usage of this software is not the responsability of the author.
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
   Thanks to first test of RF with Pifm by Oliver Mattos and Oskar Weigl 	
   INSPIRED BY THE IMPLEMENTATION OF PIFMDMA by Richard Hirst <richardghirst@gmail.com>  December 2012
   Helped by a code fragment by PE1NNZ (http://pe1nnz.nl.eu.org/2013/05/direct-ssb-generation-on-pll.html)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

//#include "utils.h"
#include "mailbox.h"
#include <getopt.h>
#include <termios.h>		//Used for UART
#include "RpiGpio.h"
#include "RpiDma.h"
#include <pthread.h>

#include "RpiTx.h"

#include <sys/prctl.h>
#include <getopt.h>
//#define PWMA_BYRANGE


//Minimum Time in us to sleep
#define KERNEL_GRANULARITY 20000 

#define SCHED_PRIORITY 30 //Linux scheduler priority. Higher = more realtime

//#define WITH_MEMORY_BUFFER


#define PROGRAM_VERSION "0.1"

#define PLL_FREQ_500MHZ		500000000	// PLLD is running at 500MHz
#define PLL_500MHZ 		0x6

#define PLL_FREQ_1GHZ             1000000000	//PLLC = 1GHZ
#define PLL_1GHZ			0x5


#define PLLFREQ_192             19200000	//PLLA = 19.2MHZ
#define PLL_192			0x1


typedef unsigned char 	uchar ;		// 8 bit
typedef unsigned short	uint16 ;	// 16 bit
typedef unsigned int	uint ;		// 32 bits

//F5OEO Variable
uint32_t PllFreq500MHZ;
uint32_t PllFreq1GHZ;
uint32_t PllFreq19MHZ;

double TuneFrequency=62500000;
unsigned char FreqDivider=2;
int DmaSampleBurstSize=1000;
int NUM_SAMPLES=NUM_SAMPLES_MAX;
//End F5OEO



char EndOfApp=0;
unsigned char loop_mode_flag=0;
char *FileName = 0;
int FileInHandle; //Handle in Transport Stream File
static void
udelay(int us)
{
	struct timespec ts = { 0, us * 1000 };

	nanosleep(&ts, NULL);
}

static void
terminate(int dummy)
{
	#ifdef WITH_MEMORY_BUFFER
	//pthread_cancel(th1);
	EndOfApp=1;
	 pthread_join(th1, NULL);
	#endif
	close(FileInHandle);
	if (dma_reg) {
		//Stop Main DMA
		dma_reg[DMA_CS+DMA_CHANNEL*0x40] = BCM2708_DMA_INT | BCM2708_DMA_END;
		udelay(100);
		dma_reg[DMA_CS+DMA_CHANNEL*0x40] = BCM2708_DMA_RESET;
		udelay(100);
		//Stop  DMA PWMFrequency
		dma_reg[DMA_CS+DMA_CHANNEL_PWMFREQUENCY*0x40] = BCM2708_DMA_INT | BCM2708_DMA_END;
		udelay(100);
		dma_reg[DMA_CS+DMA_CHANNEL_PWMFREQUENCY*0x40] = BCM2708_DMA_RESET;
		udelay(100);


		//printf("Reset DMA Done\n");
	clk_reg[GPCLK_CNTL] = 0x5A << 24  | 0 << 9 | 1 << 4 | 6; //NO MASH !!!
	udelay(500);
	gpio_reg[GPFSEL0] = (gpio_reg[GPFSEL0] & ~(7 << 12)) | (0 << 12); //DISABLE CLOCK - In case used by digilite
	clk_reg[PWMCLK_CNTL] = 0x5A000006 | (0 << 9) ;
	udelay(500);	
	clk_reg[PCMCLK_CNTL] = 0x5A000006;	
	udelay(500);
	//printf("Resetpcm Done\n");
	pwm_reg[PWM_DMAC] = 0;
	udelay(100);
	pwm_reg[PWM_CTL] = PWMCTL_CLRF;
	udelay(100);
	//printf("Reset pwm Done\n");
	}
	if (mbox.virt_addr != NULL) {
		unmapmem(mbox.virt_addr, NUM_PAGES * PAGE_SIZE);
		//printf("Unmapmem Done\n");
		mem_unlock(mbox.handle, mbox.mem_ref);
		//printf("Unmaplock Done\n");
		mem_free(mbox.handle, mbox.mem_ref);
		//printf("Unmapfree Done\n");
	}
	
	//munmap(virtbase,NUM_PAGES * PAGE_SIZE); 
	printf("END OF PiTx\n");
	exit(1);
}

static void
fatal(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	terminate(0);
}





#define DATA_FILE_SIZE 4080 // Not used anymore

void setSchedPriority(int priority) {
//In order to get the best timing at a decent queue size, we want the kernel to avoid interrupting us for long durations.
//This is done by giving our process a high priority. Note, must run as super-user for this to work.
struct sched_param sp;
sp.sched_priority=priority;
int ret;
if ((ret = pthread_setschedparam(pthread_self(), SCHED_RR, &sp))) {
printf("Warning: pthread_setschedparam (increase thread priority) returned non-zero: %i\n", ret);
}
/*
 if ((ret = pthread_setschedparam(th1, SCHED_RR, &sp))) {
printf("Warning: pthread_setschedparam (increase thread priority) returned non-zero: %i\n", ret);
}
*/
}



int SetupGpioClock(uint32_t SymbolRate,uint32_t PwmNumberStep)
{
	char MASH=1;
	 		
		/*
		gpio_reg[GPFSEL0] = (gpio_reg[GPFSEL0] & ~(7 << 12)) | (4 << 12); //ENABLE CLOCK ON GPIO CLK
		usleep(1000);
		clk_reg[GPCLK_CNTL] = 0x5A << 24 |0<<4 | PLL_1GHZ;
		usleep(1000);
		clk_reg[GPCLK_CNTL] = 0x5A << 24  | 3 << 9 | 1 << 4 | PLL_1GHZ; // MASH 1
		*/

	// ------------------- MAKE MAX OUTPUT CURRENT FOR GPIO -----------------------
	char OutputPower=7;
	pad_gpios_reg[PADS_GPIO_0] = 0x5a000000 + (OutputPower&0x7) + (1<<4) + (0<<3); // Set output power for I/Q GPIO18/GPIO19 


	//------------------- Init PCM ------------------
	pcm_reg[PCM_CS_A] = 1;				// Disable Rx+Tx, Enable PCM block
	udelay(100);
	clk_reg[PCMCLK_CNTL] = 0x5A000000|PLL_1GHZ;		// Source=PLLC (1GHHz) 
	udelay(100);
	static uint32_t FreqDividerPCM;
	static uint32_t FreqFractionnalPCM;
	int NbStepPCM = 25; // Should not exceed 1000 : 
	FreqDividerPCM=(int) ((double)PllFreq1GHZ/(SymbolRate*NbStepPCM/**PwmNumberStep*/));
	FreqFractionnalPCM=4096.0 * (((double)PllFreq1GHZ/(SymbolRate*NbStepPCM/**PwmNumberStep*/))-FreqDividerPCM);
	if((FreqDividerPCM>4096)||(FreqDividerPCM<2)) printf("Warning : SampleRate is not valid\n"); 
	clk_reg[PCMCLK_DIV] = 0x5A000000 | ((FreqDividerPCM)<<12) | FreqFractionnalPCM;

	//printf("Div PCM %d FracPCM %d\n",FreqDividerPCM,FreqFractionnalPCM);
	
	 uint32_t DelayFromSampleRate=(1e9/(SymbolRate)-(2080))/27.4;
	//uint32_t DelayFromSampleRate=(1000000.0-2080.0)/27.4425;
	//uint32_t DelayFromSampleRate=1000000;
	//uint32_t DelayFromSampleRate=round((1000000-(2300))/27.4425);
	//TimeDelay=0 : 2.08us
	//TimeDelay=10000 : 280us
	//TimdeDelay=100000 :2774us
	//TimdeDelay=1000000 : 27400 us
	//uint32_t DelayFromSampleRate=5000;
	//printf("DelaySample %d Delay %d\n",DelayFromSampleRate,(int)(27.4425*DelayFromSampleRate+2300));
		
	udelay(100);
		
	pcm_reg[PCM_TXC_A] = 0<<31 | 1<<30 | 0<<20 | 0<<16; // 1 channel, 8 bits
	udelay(100);
	
	//printf("Nb PCM STEP (<1000):%d\n",NbStepPCM);
	pcm_reg[PCM_MODE_A] = (NbStepPCM-1)<<10; // SHOULD NOT EXCEED 1000 !!!
	udelay(100);
	pcm_reg[PCM_CS_A] |= 1<<4 | 1<<3;		// Clear FIFOs
	udelay(100);
	pcm_reg[PCM_DREQ_A] = 64<<24 | /*64<<8 |*/ 64<<8 ;		//TX Fifo PCM=64 DMA Req when one slot is free?
	udelay(100);
	pcm_reg[PCM_CS_A] |= 1<<9;			// Enable DMA
	udelay(1000);
	clk_reg[PCMCLK_CNTL] = 0x5A000010 | PLL_1GHZ	/*PLL_1GHZ*/;		// Source=PLLC and enable
	udelay(100);
	pcm_reg[PCM_CS_A] |= 1<<2; //START TX PCM

	// FIN PCM

	//INIT PWM in Serial Mode
	gpioSetMode(18, 2); /* set to ALT5, PWM1 : RF On PIN */	

	pwm_reg[PWM_CTL] = 0;
	clk_reg[PWMCLK_CNTL] = 0x5A000000 | (0 << 9) |PLL_1GHZ ;
	udelay(300);
	clk_reg[PWMCLK_DIV] = 0x5A000000 | (2<<12); //WILL BE UPDATED BY DMA
	udelay(300);
	clk_reg[PWMCLK_CNTL] = 0x5A000010 | (MASH << 9) | PLL_1GHZ; //MASH3 : A TESTER SI MIEUX en MASH1
	//MASH 3 doesnt seem work above 80MHZ, back to MASH1
	pwm_reg[PWM_RNG1] = 32;// 250 -> 8KHZ
	udelay(100);
	pwm_reg[PWM_RNG2] = 32;// 32 Mandatory for Serial Mode without gap
	
	pwm_reg[PWM_FIFO]=0xAAAAAAAA;
	pwm_reg[PWM_DMAC] = PWMDMAC_ENAB | PWMDMAC_THRSHLD;
	udelay(100);
	pwm_reg[PWM_CTL] = PWMCTL_CLRF;
	udelay(100);
	pwm_reg[PWM_CTL] =   PWMCTL_USEF1| PWMCTL_MODE1| PWMCTL_PWEN1|PWMCTL_RPTL1; //PWM0 in Repeat mode 
	// FIN INIT PWM

	
	ctl = (struct control_data_s *)virtbase; // Struct ctl is mapped to the memory allocated by RpiDMA (Mailbox)
	dma_cb_t *cbp = ctl->cb;

	uint32_t phys_pwm_fifo_addr = 0x7e20c000 + 0x18;//PWM Fifo
	uint32_t phys_pwm_range_addr = 0x7e20c000 + 0x10;//PWM Range
	uint32_t phys_clock_div_addr = 0x7e101074;//CLOCK Frequency Setting
	uint32_t phys_pwm_clock_div_addr = 0x7e1010a4; //CLK PWM
	uint32_t phys_gpio_set_addr = 0x7e20001c;
	uint32_t phys_gpio_clear_addr = 0x7e200028;
	uint32_t dummy_gpio = 0x7e20b000;
	uint32_t phys_pcm_fifo_addr = 0x7e203004;
				      
	uint32_t phys_pcm_clock =     0x7e10109c ;
	uint32_t phys_DmaPwmfRegCtrl = 0x7e007000+DMA_CHANNEL_PWMFREQUENCY*0x100+0x4;
	int samplecnt;
	
			
			gpioSetMode(27,1); // OSCILO SUR PIN 27
			ctl->GpioDebugSampleRate=1<<27; // GPIO 27/ PIN 13 HEADER IS DEBUG SIGNAL OF SAMPLERATE
			ctl->DmaPwmfControlRegister=mem_virt_to_phys((void *)(ctl->cbdma2) ); 
	

			for (samplecnt = 0; samplecnt <  NUM_SAMPLES ; samplecnt++)
			{
				
				//ctl->sample[samplecnt].WaitForThisSample=DelayFromSampleRate;
				ctl->sample[samplecnt].WaitForThisSample=0x5A000000 | ((FreqDividerPCM)<<12) | FreqFractionnalPCM;
				//Restart PWMF at start
				cbp->info = 0;// BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
				cbp->src = mem_virt_to_phys(&ctl->DmaPwmfControlRegister);  
				cbp->dst = phys_DmaPwmfRegCtrl;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);		
				cbp++;
				
				
				
				#ifdef PWMA_BYRANGE
				//Set Amplitude by writing to PWM_SERIAL via Range	
				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].Amplitude1);  
				cbp->dst = phys_pwm_range_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);		
				cbp++;
				#else
				//Set Amplitude by writing to PWM_SERIAL via Patern	
				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP  ;
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].Amplitude1);  
				cbp->dst = phys_pwm_fifo_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);		
				cbp++;
				#endif


				//SET Frequency1 to DMA 2

				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | (0x0<< 21);
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].Frequency1); 
				cbp->dst = mem_virt_to_phys(&ctl->SharedFrequency1);
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
				
				//SET Frequency2 to DMA 2

				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | (0x0<< 21);
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].Frequency2); 
				cbp->dst = mem_virt_to_phys(&ctl->SharedFrequency2);
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
				
				//SET PWMF1 DMA 2

				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | (0x0<< 21);
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].PWMF1); 
				cbp->dst = mem_virt_to_phys(&ctl->cbdma2[2].length);
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;

				//SET PWMF2 DMA 2

				cbp->info =0;// BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | (0x0<< 21);
				cbp->src = mem_virt_to_phys(&ctl->sample[samplecnt].PWMF2); 
				cbp->dst = mem_virt_to_phys(&ctl->cbdma2[5].length);
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
		
				//SET GPIO DEBGUB TOGGLE/CLEAR/SET
				cbp->info = 0;//BCM2708_DMA_NO_WIDE_BURSTS |  BCM2708_DMA_WAIT_RESP | (0x0<< 21);
				cbp->src = mem_virt_to_phys(&ctl->GpioDebugSampleRate);
				if((samplecnt%2)==0) 
					cbp->dst = phys_gpio_clear_addr;
				else
					cbp->dst = phys_gpio_set_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;

			

				//SET PCM_FIFO TO WAIT	

				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP|BCM2708_DMA_D_DREQ | BCM2708_DMA_PER_MAP(2);
				cbp->src = mem_virt_to_phys(virtbase); //Dummy DATA
				cbp->dst = phys_pcm_fifo_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
				
				/*cbp->info =BCM2708_DMA_NO_WIDE_BURSTS;
				cbp->src = mem_virt_to_phys(virtbase); 
				cbp->dst = dummy_gpio;
				cbp->length = ctl->sample[samplecnt].WaitForThisSample;
				cbp->stride =0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;*/
				
				
			}
			cbp--;
			cbp->next = mem_virt_to_phys((void*)virtbase);

// *************************************** DMA CHANNEL 2 : PWM FREQUENCY *********************************************
				
				cbp = ctl->cbdma2;

				//printf("DMA2 %x\n",mem_virt_to_phys(&ctl->cb[NUM_CBS_MAIN]));

				gpioSetMode(17,1); // OSCILO SUR PIN 17
				ctl->GpioDebugPWMF=1<<17; // GPIO 17/ PIN 11 HEADER IS DEBUG SIGNAL OF PWMFREQUENCY
				


//@ = CBS_MAIN+0 1280 ns for 2 instructions
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS /*| BCM2708_DMA_WAIT_RESP | */;
				cbp->src = mem_virt_to_phys(&ctl->GpioDebugPWMF); //Clear
				cbp->dst = phys_gpio_clear_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
//@ = CBS_MAIN+1
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS  /*| BCM2708_DMA_WAIT_RESP | */;
				cbp->src = mem_virt_to_phys(&ctl->SharedFrequency1); //Be updated by Main DMA to ctl->sample[samplecnt].Frequency1
				cbp->dst = phys_pwm_clock_div_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
//@ = CBS_MAIN+2	Length*27ns			
				cbp->info =BCM2708_DMA_NO_WIDE_BURSTS;
				cbp->src = mem_virt_to_phys(virtbase); 
				cbp->dst = dummy_gpio;
				cbp->length = ctl->sample[0].PWMF1; //Be updated by main DMA
				cbp->stride = 4;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
//@ = CBS_MAIN+3
				//** PWM LOW
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS /*| BCM2708_DMA_WAIT_RESP |*/ ; //A
				cbp->src = mem_virt_to_phys(&ctl->GpioDebugPWMF); //Set
				cbp->dst = phys_gpio_set_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
//@ = CBS_MAIN+4
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS /*| BCM2708_DMA_WAIT_RESP | */;
				cbp->src = mem_virt_to_phys(&ctl->SharedFrequency2); //Be updated by Main DMA to ctl->sample[samplecnt].Frequency2
				cbp->dst = phys_pwm_clock_div_addr;
				cbp->length = 4;
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(cbp + 1);
				cbp++;
//@ = CBS_MAIN+5				
				cbp->info = BCM2708_DMA_NO_WIDE_BURSTS;
				cbp->src = mem_virt_to_phys(virtbase); 
				cbp->dst = dummy_gpio;
				cbp->length = 500; //Be updated by main DMA
				cbp->stride = 0;
				cbp->next = mem_virt_to_phys(ctl->cbdma2); // Loop to begin of DMA 2
				
				


	// ------------------------------ END DMA INIT ---------------------------------
	

	dma_reg[DMA_CS+DMA_CHANNEL*0x40] = BCM2708_DMA_RESET;
	udelay(1000);
	dma_reg[DMA_CS+DMA_CHANNEL*0x40] = BCM2708_DMA_INT | BCM2708_DMA_END;
	udelay(100);
	dma_reg[DMA_CONBLK_AD+DMA_CHANNEL*0x40]=mem_virt_to_phys((void *)virtbase );
	udelay(100);
	dma_reg[DMA_DEBUG+DMA_CHANNEL*0x40] = 7; // clear debug error flags
	udelay(100);
	// START DMA will be later when data coming in 
	//dma_reg[DMA_CS+DMA_CHANNEL*0x40] = 0x10FF0001;	// go, mid priority, wait for outstanding writes :7 Seems Max Priorit
	//WAIT FOR OUTSTADING_WRITE make 50ns de plus pour ecrire dans un registre
	//printf("END INIT SSTV\n");
	dma_reg[DMA_CS+DMA_CHANNEL_PWMFREQUENCY*0x40] = BCM2708_DMA_RESET;
	udelay(1000);
	dma_reg[DMA_CS+DMA_CHANNEL_PWMFREQUENCY*0x40] = BCM2708_DMA_INT | BCM2708_DMA_END;
	udelay(100);
	dma_reg[DMA_CONBLK_AD+DMA_CHANNEL_PWMFREQUENCY*0x40]=mem_virt_to_phys((void *)(ctl->cbdma2) );
	udelay(100);
	dma_reg[DMA_DEBUG+DMA_CHANNEL_PWMFREQUENCY*0x40] = 7; // clear debug error flags

	
	
 	return 1;		
}







#define ln(x) (log(x)/log(2.718281828459045235f))


int arctan2(int y, int x) // Should be replaced with fast_atan2 from rtl_fm
{
   int abs_y = abs(y);
   int angle;
   if((x==0)&&(y==0)) return 0;
   if(x >= 0){
      angle = 45 - 45 * (x - abs_y) / ((x + abs_y)==0?1:(x + abs_y));
   } else {
      angle = 135 - 45 * (x + abs_y) / ((abs_y - x)==0?1:(abs_y - x));
   }
   return (y < 0) ? -angle : angle; // negate if in quad III or IV
}





void IQToFreqAmp(int I,int Q,double *Frequency,int *Amp,int SampleRate)
{
	float phase;
	static float prev_phase = 0;
	
	*Amp=round(sqrt( I*I + Q*Q)/sqrt(2));
	//*Amp=*Amp*3; // Amp*5 pour la voix !! To be tested more
	if(*Amp>32767)
	{
		printf("!");
		*Amp=32767; //Overload
	}

	phase = M_PI + ((float)arctan2(I,Q)) * M_PI/180.0f;
  	 float dp = phase - prev_phase;
   	if(dp < 0) dp = dp + 2*M_PI;
    
   
   	*Frequency = dp*SampleRate/(2.0f*M_PI);
	prev_phase = phase;
	//printf("I=%d Q=%d Amp=%d Freq=%d\n",I,Q,*Amp,*Frequency);
	
}


void FrequencyAmplitudeToRegister(double TuneFrequency,uint32_t Amplitude,int NoSample,int MaxDelay,uint32_t WaitNanoSecond,uint32_t SampleRate)
{
				static char ShowInfo=1;				
				uint32_t FreqDivider;
				uint32_t FreqFractionnal;
				static uint32_t CompteurDebug=0;
				int PwmNumberStep;
				CompteurDebug++;
				static uint32_t TabPwmAmplitude[18]={0x00000000,
								      0x80000000,0xA0000000,0xA8000000,0xAA000000,
								      0xAA800000,0xAAA00000,0xAAA80000,0xAAAA0000,
							              0xAAAA8000,0xAAAAA000,0xAAAAA800,0xAAAAAA00,
								      0xAAAAAA80,0xAAAAAAA0,0xAAAAAAA8,0xAAAAAAAA,0xAAAAAAAA};
				static double FixedNumberStepForGpio=(1280.0)/27.0; // Gpio set is 1280ns, Wait is Number of 27ns
				if(WaitNanoSecond!=0)
				{
					uint32_t ReduceWait;
					int Divider=1;
					do
					{
						ReduceWait=WaitNanoSecond/Divider; //Max cycle =30us
						Divider++;
					}
					while(ReduceWait>MaxDelay); // Should be in function of resolution in HZ
					PwmNumberStep=(ReduceWait-2096)/(27.44);//4 cycles
					
					
					//printf("NumberStep=%d Reduce=%d\n",PwmNumberStep,ReduceWait);
				}
				else
				{
					PwmNumberStep=(1000000.0/((double)SampleRate/1000.0)-2095)/27.44;//should be samplerate instead of 48
					//printf("NumberStep=%d\n",PwmNumberStep);
				}
				ctl = (struct control_data_s *)virtbase; // Struct ctl is mapped to the memory allocated by RpiDMA (Mailbox)
				dma_cb_t *cbp = ctl->cb+NoSample*CBS_SIZE_BY_SAMPLE;
				
				TuneFrequency*=2.0; //Because of pattern 10
				FreqDivider=(int) ((double)PllFreq1GHZ/TuneFrequency);
				FreqFractionnal=4096.0 * (((double)PllFreq1GHZ/TuneFrequency)-FreqDivider);
				
				uint32_t FreqDividerf1=(FreqFractionnal!=4095)?FreqDivider:FreqDivider+1;				
				uint32_t FreqFractionnalf1=(FreqFractionnal!=4095)?FreqFractionnal+1:0;

				double FreqTuning=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096);

				double f1=PllFreq1GHZ/(FreqDividerf1+(double)FreqFractionnalf1/4096);
				double f2=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096); // f1 is the higher frequency
				double FreqStep=f2-f1;
				
				
				
				double	fPWMFrequency=((FreqTuning-TuneFrequency)*1.05*(double)(PwmNumberStep+FixedNumberStepForGpio)/FreqStep);
				int PWMFrequency=round(fPWMFrequency);
				
				if(PWMFrequency<=FixedNumberStepForGpio) 
				{
					//FreqDividerf1=(FreqFractionnal!=4093)?FreqDivider:FreqDivider+1;				
					//FreqFractionnalf1=(FreqFractionnal!=4093)?FreqFractionnal+2:0;
					
					FreqDivider=(FreqFractionnal!=0)?FreqDivider:FreqDivider-1;				
					FreqFractionnal=(FreqFractionnal!=0)?FreqFractionnal-1:4095;

					f1=PllFreq1GHZ/(FreqDividerf1+(double)FreqFractionnalf1/4096.0);
					f2=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096.0);
					FreqStep=f2-f1;
					FreqTuning=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096.0);
					fPWMFrequency=((FreqTuning-TuneFrequency)*1.05*(double)(PwmNumberStep+FixedNumberStepForGpio)/FreqStep);
					PWMFrequency=round(fPWMFrequency);//+20 pour raccord
					//if((CompteurDebug%200)==0) printf("Corrected FreqStep %f PWMFrequency %d %f %f %d\n",FreqStep,PWMFrequency,f1,f2,FreqFractionnal);
					if(PWMFrequency<0) PWMFrequency=0;
					
				}
				else
					if(PWMFrequency>=PwmNumberStep) 
					{
						FreqDividerf1=(FreqFractionnal!=4094)?FreqDivider:FreqDivider+1;				
						FreqFractionnalf1=(FreqFractionnal!=4094)?FreqFractionnal+2:0;
					
						f1=PllFreq1GHZ/(FreqDividerf1+(double)FreqFractionnalf1/4096.0);
						f2=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096.0);
						FreqStep=f2-f1;
						FreqTuning=PllFreq1GHZ/(FreqDivider+(double)FreqFractionnal/4096.0);
						fPWMFrequency=((FreqTuning-TuneFrequency)*1.05*(double)(PwmNumberStep+FixedNumberStepForGpio)/FreqStep);
						PWMFrequency=round(fPWMFrequency);
						//if((CompteurDebug%200)==0) printf("Up Corrected FreqStep %f PWMFrequency %d\n",FreqStep,PWMFrequency);
						if(PWMFrequency<0) PWMFrequency=0;
					
					

					}
				ctl->sample[NoSample].Frequency1=0x5A000000 | (FreqDividerf1<<12) | (FreqFractionnalf1);
				ctl->sample[NoSample].Frequency2=0x5A000000 | (FreqDivider<<12) | (FreqFractionnal); 
				
				//PWM Amplitude via patern
				/*	
				int IntAmplitude=(Amplitude*16.0)/32767.0; // Convert to 16 amplitude step		
				ctl->sample[NoSample].Amplitude1=TabPwmAmplitude[IntAmplitude];
				ctl->sample[NoSample].Amplitude2=TabPwmAmplitude[IntAmplitude+1]; // Seems not very cool : lot of harmonics
				*/
				
				#ifdef PWMA_BYRANGE
				//PWM Amplitude via range
				float fAmplitude=Amplitude*1.0/32767.0;	
				//if(fAmplitude<0.1) fAmplitude=0.1;
				//if(fAmplitude>1.0) fAmplitude=1.0;
				int NbSilenceStep=32+(16.0/fAmplitude-16.0)*2;
				if (NbSilenceStep>(PwmNumberStep)) NbSilenceStep=PwmNumberStep-32;	
				ctl->sample[NoSample].Amplitude1=NbSilenceStep;
				//printf("Amplitude %d Amplitude %f NbStep=%d\n",Amplitude,fAmplitude,NbSilenceStep);
				//ctl->sample[NoSample].Amplitude2=TabPwmAmplitude[IntAmplitude+1]; // Seems not very cool : lot of harmonics
				#else
				//PWM Amplitude via patern
				Amplitude=(Amplitude>32767)?32767:Amplitude;	
				int IntAmplitude=(Amplitude*16.0)/32767.0; // Convert to 16 amplitude step		
				ctl->sample[NoSample].Amplitude1=TabPwmAmplitude[IntAmplitude];//TEST +1
				ctl->sample[NoSample].Amplitude2=TabPwmAmplitude[IntAmplitude+1]; // Seems not very cool : lot of harmonics
				
				#endif
				
				
				
				
				ctl->sample[NoSample].PWMF1=PWMFrequency-FixedNumberStepForGpio;
				ctl->sample[NoSample].PWMF2=PwmNumberStep-(PWMFrequency-FixedNumberStepForGpio);
				
				if(ShowInfo==1)
				{
					printf("Resolution(Hz)=%f\n",FreqStep/(PwmNumberStep+FixedNumberStepForGpio));
					ShowInfo=0;
				}

				//WaitNanoSecond=5600;
				if(WaitNanoSecond!=0) //ModeRF
				{
					
					int NbStepPCM = 25; // Should not exceed 1000
					
					
					static int Relicat=0;

					cbp+=7; // Set a repeat to wait PCM
					uint32_t NbRepeat=WaitNanoSecond/1000; // 1GHZ/(SAMPLERATE*NbStepPCM)
					Relicat+=WaitNanoSecond%1000;
					if(Relicat>=1000) 
					{
						Relicat=Relicat-1000;
						NbRepeat+=1;
					}
					
					
					cbp->length=4*(NbRepeat); //-> OK
					
					//cbp->info|=((cbp->info&~(0x1F<<21))|(FineRepeat<<21));
					//printf("WaitNano =%d NbRepeat %d FineRepeat%d\n",WaitNanoSecond,NbRepeat,Relicat);
					
				}
				





				/*if(WaitNanoSecond!=0) //ModeRF
				{
					//uint32_t DelayFromSampleRate=(1e9/(SymbolRate)-(2080))/27.4;
					uint32_t DelayFromWait=round((WaitNanoSecond-(1140+1080))/(double)27.4425);
					//printf("WaitNano %d, Delayfromwait%d\n",WaitNanoSecond,DelayFromWait);
					cbp+=6; // To be on right CBP
					cbp->length=DelayFromWait;
				}*/
				
				//printf("PWMF1 %d PWMF2 %d\n",ctl->sample[NoSample].PWMF1,ctl->sample[NoSample].PWMF2);
				//ctl->sample[NoSample].PWMF1=(CompteurDebug/1000)%600;
				//ctl->sample[NoSample].PWMF2=600-(CompteurDebug/1000)%600;
				//if((CompteurDebug%100)==0)printf("Request =%f Set %f Error =%f PWM %d\n",TuneFrequency,FreqTuning-(FreqStep*PWMFrequency/(float)PwmNumberStep),TuneFrequency-(FreqTuning-(FreqStep*PWMFrequency/(float)PwmNumberStep)),PWMFrequency);
				//if((CompteurDebug%100)==0)printf("Request =%f Div = %d/%d Frac=%d / %d Tuning = %f Step=%f fPWMFrequency %f FreqFreqPWM=%d f1 %f f2 %f %x %x : %f \n",		       TuneFrequency,FreqDivider,FreqDividerf1,FreqFractionnal,FreqFractionnalf1,FreqTuning,FreqStep,fPWMFrequency,PWMFrequency,f1,f2,ctl->sample[NoSample].Frequency1,ctl->sample[NoSample].Frequency2,FreqTuning-(FreqStep*PWMFrequency/(float)PwmNumberStep));
				
}


int pitx_init(int SampleRate)
{
	InitGpio();
	InitDma(terminate);
	SetupGpioClock(SampleRate,NBSAMPLES_PWM_FREQ_MAX);
	//printf("Timing : 1 cyle=%dns 1sample=%dns\n",NBSAMPLES_PWM_FREQ_MAX*400*3,(int)(1e9/(float)SampleRate));
	return 1;
}


void print_usage()
{

fprintf(stderr,\
"\nrpitx -%s\n\
Usage:\nrpitx [-i File Input][-m ModeInput] [-f frequency output] [-s Samplerate] [-l] [-p ppm] [-h] \n\
-m            {IQ(FileInput is a Stereo Wav contains I on left Channel, Q on right channel)}\n\
              {IQFLOAT(FileInput is a Raw float interlaced I,Q)}\n\
              {RF(FileInput is a (double)Frequency,Time in nanoseconds}\n\
       	      {RFA(FileInput is a (double)Frequency,(int)Time in nanoseconds,(float)Amplitude}\n\
	      {VFO (constant frequency)}\n\
-i            path to File Input \n\
-f float      frequency to output on GPIO_18 pin 12 in khz : (130 kHz to 750 MHz),\n\
-l            loop mode for file input\n\
-p float      frequency correction in parts per million (ppm), positive or negative, for calibration, default 0.\n\
-d int 	      DMABurstSize (default 1000) but for very short message, could be decrease\n\
-h            help (this help).\n\
\n",\
PROGRAM_VERSION);

} /* end function print_usage */


double GlobalTuningFrequency;
int HarmonicNumber =1;
int pitx_SetTuneFrequency(double Frequency)
{
	if(Frequency>250000000) //Above 250MHZ, use 3rd harmonic
		{
			GlobalTuningFrequency=Frequency/3;
			HarmonicNumber=3;
			printf("\n Warning : Using harmonic 3\n");
		}
	else
	{
		GlobalTuningFrequency=Frequency;
	}
	return 1;
}

int
main(int argc, char **argv)
{
	int a;
	int i;
	//char pagemap_fn[64];
	
	int OffsetModulation=10000;//TBR
	int MicGain=100;
	//unsigned char *data;
	
	#define MODE_IQ 0
	#define MODE_RF 1
	#define MODE_RFA 2
	#define MODE_IQ_FLOAT 3
	#define MODE_VFO 4
        char Mode=0;
	int SampleRate=48000;
	float SetFrequency=1e6;//1MHZ
	float ppmpll=0.0;

	//Specific to ModeIQ
	static signed short *IQArray=NULL;

	//Specific to ModeIQ_FLOAT
	static float *IQFloatArray=NULL;

	//Specific to Mode RF 
	typedef struct {
		double Frequency;
		uint32_t WaitForThisSample;
	} samplerf_t;
	samplerf_t *TabRfSample=NULL;
		
	fprintf(stdout,"rpitx Version %s compiled %s (F5OEO Evariste) running on ",PROGRAM_VERSION,__DATE__);
	
	Mode=MODE_IQ; //By default

	int anyargs = 0;

	while(1)
	{
	a = getopt(argc, argv, "i:f:m:s:p:hld:");
	
	if(a == -1) 
	{
		if(anyargs) break;
		else a='h'; //print usage and exit
	}
	anyargs = 1;	

	switch(a)
		{
		case 'i': // Frequency
			FileName = optarg;
			break;
		case 'f': // Frequency
			SetFrequency = atof(optarg);
			break;
		case 'm': // Mode (IQ,IQFLOAT,RF,RFA)
			if(strcmp("IQ",optarg)==0) Mode=MODE_IQ;
			if(strcmp("RF",optarg)==0) Mode=MODE_RF;	
			if(strcmp("RFA",optarg)==0) Mode=MODE_RFA;
			if(strcmp("IQFLOAT",optarg)==0) Mode=MODE_IQ_FLOAT;
			if(strcmp("VFO",optarg)==0) Mode=MODE_VFO;
			break;
		case 's': // SampleRate (Only needeed in IQ mode)
			SampleRate = atoi(optarg);
			break;
		case 'p':  //	ppmcorrection
			ppmpll = atof(optarg);
			
			break;
		case 'h': // help
			print_usage();
			exit(1);
			break;
		case 'l': // loop mode
			loop_mode_flag = 1;
			break;
		case 'd': // loop mode
			DmaSampleBurstSize = atoi(optarg);
			NUM_SAMPLES=4*DmaSampleBurstSize;
			break;
        	case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 				{
 				fprintf(stderr, "rpitx: unknown option `-%c'.\n", optopt);
 				}
			else
				{
				fprintf(stderr, "rpitx: unknown option character `\\x%x'.\n", optopt);
				}
			print_usage();

			exit(1);
			break;			
		default:
			print_usage();
			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */



	// Init Plls Frequency using ppm (or default)

	PllFreq500MHZ=PLL_FREQ_500MHZ;
	PllFreq500MHZ+=PllFreq500MHZ * (ppmpll / 1000000.0);

	PllFreq1GHZ=PLL_FREQ_1GHZ;
	PllFreq1GHZ+=PllFreq1GHZ * (ppmpll / 1000000.0);

	PllFreq19MHZ=PLLFREQ_192;
	PllFreq19MHZ+=PllFreq19MHZ * (ppmpll / 1000000.0);

	//End of Init Plls
	//Open File Input for modes which need it
	if((Mode==MODE_IQ)||(Mode==MODE_IQ_FLOAT)||(Mode==MODE_RF)||(Mode==MODE_RFA))
	{
		if(FileName && strcmp(FileName,"-")==0)
		{
			FileInHandle = STDIN_FILENO;
		}
		else FileInHandle = open(FileName, 'r');
		if (FileInHandle < 0)
		{
			fatal("Failed to read Filein %s\n",FileName);
		}
	}

	if(Mode==MODE_IQ)
	{
		IQArray=malloc(DmaSampleBurstSize*2*sizeof(signed short)); // TODO A FREE AT THE END OF SOFTWARE
		char dummyheader[44];
		read(FileInHandle,dummyheader,44);
		
	} 
	if(Mode==MODE_IQ_FLOAT)
	{
		IQFloatArray=malloc(DmaSampleBurstSize*2*sizeof(float)); // TODO A FREE AT THE END OF SOFTWARE
	}
	if((Mode==MODE_RF)||(Mode==MODE_RFA))
	{
		TabRfSample=malloc(DmaSampleBurstSize*sizeof(samplerf_t));
		SampleRate=1000000L;
	}
	if(Mode==MODE_VFO)
		SampleRate=1000000L;

	if(Mode==MODE_IQ)
	{
		printf(" Frequency=%f ",GlobalTuningFrequency);
		printf(" SampleRate=%d ",SampleRate);	
	}

	pitx_init(SampleRate);
	pitx_SetTuneFrequency(SetFrequency*1000.0);
	

	
	

	
	

static volatile uint32_t cur_cb,last_cb;
int last_sample;
int this_sample; 
int free_slots;
//int SumDelay=0;
	
	
	
	
long int start_time;
static long time_difference=0;
struct timespec gettime_now;



	cur_cb = (uint32_t)virtbase+ (NUM_SAMPLES-DmaSampleBurstSize)* sizeof(dma_cb_t) *CBS_SIZE_BY_SAMPLE;
	
	last_cb=(uint32_t)virtbase /*+ 965* sizeof(dma_cb_t) *CBS_SIZE_BY_SAMPLE*/ ;


dma_reg[DMA_CONBLK_AD+DMA_CHANNEL*0x40]=mem_virt_to_phys((void*)cur_cb);


unsigned char Init=1;


// -----------------------------------------------------------------







for (;;) 
	{
			int TimeToSleep;
			static int StatusCompteur=0;
			
			cur_cb = mem_phys_to_virt((uint32_t)(dma_reg[DMA_CONBLK_AD+DMA_CHANNEL*0x40]));
			this_sample = (cur_cb - (uint32_t)virtbase) / (sizeof(dma_cb_t) * CBS_SIZE_BY_SAMPLE);
			last_sample = (last_cb - (uint32_t)virtbase) / (sizeof(dma_cb_t) * CBS_SIZE_BY_SAMPLE);
			free_slots = this_sample - last_sample;
			if (free_slots < 0) // WARNING : ORIGINAL CODE WAS < strictly
				free_slots += NUM_SAMPLES;
				
			//printf("last_sample %lx cur_cb %lx FreeSlots = %d Time to sleep=%d\n",last_sample,cur_cb,free_slots,TimeToSleep);
			
			if(Init==0)
			{
				
				TimeToSleep=(1e6*(NUM_SAMPLES-free_slots*2))/SampleRate; // Time to sleep in us
				//printf("TimeToSleep%d\n",TimeToSleep);
			}
			else
				TimeToSleep=1000;
			
	
			
			//printf("Buffer Available=%d\n",BufferAvailable());
			
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			start_time = gettime_now.tv_nsec;		
			if(TimeToSleep>=(2200+KERNEL_GRANULARITY)) // 2ms : Time to process File/Canal Coding
			{
				
				udelay(TimeToSleep-(2200+KERNEL_GRANULARITY));
				TimeToSleep=0;
			}
	
			else
			{
				
				//udelay(TimeToSleep);
				sched_yield();
				//TimeToSleep=0;
				//if(free_slots>(NUM_SAMPLES*9/10))
					 //printf("Buffer nearly empty...%d/%d\n",free_slots,NUM_SAMPLES);
			}
			
			
			static int free_slots_now;
			cur_cb = mem_phys_to_virt(dma_reg[DMA_CONBLK_AD+DMA_CHANNEL*0x40]);
			this_sample = (cur_cb - (uint32_t)virtbase) / (sizeof(dma_cb_t) * CBS_SIZE_BY_SAMPLE);
			last_sample = (last_cb - (uint32_t)virtbase) / (sizeof(dma_cb_t) * CBS_SIZE_BY_SAMPLE);
			free_slots_now = this_sample - last_sample;
			if (free_slots_now < 0) // WARNING : ORIGINAL CODE WAS < strictly
				free_slots_now += NUM_SAMPLES;
			
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			time_difference = gettime_now.tv_nsec - start_time;
			if(time_difference<0) time_difference+=1E9;
			
			if(StatusCompteur%500==0)
			{ 
				
			
			
			//printf(" DiffTime = %ld FreeSlot %d FreeSlotDiff=%d Bitrate : %f\n",time_difference,free_slots_now,free_slots_now-free_slots,(1e9*(free_slots_now-free_slots))/(float)time_difference);	
			}	
			StatusCompteur++;
			free_slots=free_slots_now;
			// FIX IT : Max(freeslot et Numsample/8)
			if((Init==1)&&(free_slots < DmaSampleBurstSize /*NUM_SAMPLES/8*/))
			{
				printf("****** STARTING TRANSMIT ********\n");
				dma_reg[DMA_CONBLK_AD+DMA_CHANNEL*0x40]=mem_virt_to_phys((void*)virtbase );
				usleep(100);
				//Start DMA PWMFrequency
				
				dma_reg[DMA_CS+DMA_CHANNEL_PWMFREQUENCY*0x40] = 0x10880001;				

				//Start Main DMA
				dma_reg[DMA_CS+DMA_CHANNEL*0x40] = 0x10880001;	// go, mid priority, wait for outstanding writes :7 Seems Max Priority
				

				Init=0;
				
				continue;
				
			}
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			start_time = gettime_now.tv_nsec;
			
		
			
			

			if ((free_slots>=DmaSampleBurstSize)) 
			{
				// *************************************** MODE IQ **************************************************
				if(Mode==MODE_IQ)
				{
					int NbRead=0;
					static int Max=0;
					static int Min=32767;
					static int CompteSample=0;
					CompteSample++;
					NbRead=read(FileInHandle,IQArray,DmaSampleBurstSize*2*2/*SHORT I,SHORT Q*/);
				
					
					if(NbRead!=DmaSampleBurstSize*2*2) 
					{
						if(loop_mode_flag==1)
						{
							printf("Looping FileIn\n");
							close(FileInHandle);
							FileInHandle = open(FileName, 'r');
							char dummyheader[44];
							read(FileInHandle,dummyheader,44);
							NbRead=read(FileInHandle,IQArray,DmaSampleBurstSize*2*2);
						}
						else
							terminate(0);
					
					}
				
					for(i=0;i<DmaSampleBurstSize;i++)
					{
							
						//static float samplerate=48000;
						static int amp;
						static double df;
					
						int CorrectionRpiFrequency=-1000; //TODO PPM / Offset=1KHZ at 144MHZ
					
						CompteSample++;
						//printf("i%d q%d\n",IQArray[2*i],IQArray[2*i+1]);
					
						IQToFreqAmp(IQArray[2*i+1],IQArray[2*i],&df,&amp,SampleRate);

						 double A = 87.7f; // compression parameter
						double ampf=amp/32767.0;
      						 ampf = (fabs(ampf) < 1.0f/A) ? A*fabs(ampf)/(1.0f+ln(A)) : (1.0f+ln(A*fabs(ampf)))/(1.0f+ln(A)); //compand
						amp= (int)(round(ampf * 32767.0f)) ;
						if(amp>Max) Max=amp;
						if(amp<Min) Min=amp;	
						if((CompteSample%4800)==0)
						{
							//printf("%d\n",((CompteSample/48000)*1024)%32767);
							printf("Amp %d Freq %f MinAmp %d MaxAmp %d\n",amp,df,Min,Max);
							// printf(".");
							fflush(stdout);
						}
						// TEST - WARNING, REMOVE FOR RELEASE 
						//amp=((CompteSample/48000)*1024)%32767;
						//df=0;
						//amp=amp*10;
						//amp=32767;
						//if(df<OffsetModulation+100) amp=0;
						//
						FrequencyAmplitudeToRegister((GlobalTuningFrequency-OffsetModulation+df)/HarmonicNumber,amp,last_sample++,680,0,SampleRate);
						// !!!!!!!!!!!!!!!!!!!! 680 is for 48KHZ , should be adpated !!!!!!!!!!!!!!!!!

						free_slots--;
						if (last_sample == NUM_SAMPLES)	last_sample = 0;
					}
				}
				// *************************************** MODE IQ FLOAT**************************************************
				if(Mode==MODE_IQ_FLOAT)
				{
					int NbRead=0;
					static int Max=0;
					static int Min=32767;
					static int CompteSample=0;
					CompteSample++;
					NbRead=read(FileInHandle,IQFloatArray,DmaSampleBurstSize*2*sizeof(float));
				
					if(NbRead!=DmaSampleBurstSize*2*sizeof(float)) 
					{
						if(loop_mode_flag==1)
						{
							printf("Looping FileIn\n");
							close(FileInHandle);
							FileInHandle = open(FileName, 'r');
						}
						else if (FileInHandle != STDIN_FILENO) 
							terminate(0);
						
					
					}
				
					for(i=0;i<DmaSampleBurstSize;i++)
					{
							
						//static float samplerate=48000;
						static int amp;
						static double df;
					
						int CorrectionRpiFrequency=-1000; //TODO PPM / Offset=1KHZ at 144MHZ
					
						CompteSample++;
						//printf("i%d q%d\n",IQArray[2*i],IQArray[2*i+1]);
					
						IQToFreqAmp(IQFloatArray[2*i+1]*32767,IQFloatArray[2*i]*32767,&df,&amp,SampleRate);

						
						if(amp>Max) Max=amp;
						if(amp<Min) Min=amp;
						/*	
						if((CompteSample%4800)==0)
						{
							//printf("%d\n",((CompteSample/48000)*1024)%32767);
							printf("Amp %d Freq %f MinAmp %d MaxAmp %d\n",amp,df,Min,Max);
							// printf(".");
							fflush(stdout);
						}
						*/
						// TEST - WARNING, REMOVE FOR RELEASE 
						//amp=((CompteSample/48000)*1024)%32767;
						//df=0;
						//amp=amp*10;
						//amp=32767;
						//if(df<OffsetModulation+100) amp=0;
						//
						//amp=32767;
						//if(df>SampleRate/2) df=SampleRate/2-df;
						FrequencyAmplitudeToRegister((GlobalTuningFrequency-OffsetModulation+df)/HarmonicNumber,amp,last_sample++,680,0,SampleRate);
						// !!!!!!!!!!!!!!!!!!!! 680 is for 48KHZ , should be adpated !!!!!!!!!!!!!!!!!

						free_slots--;
						if (last_sample == NUM_SAMPLES)	last_sample = 0;
					}
				}
		// *************************************** MODE RF **************************************************
				if(Mode==MODE_RF)
				{
					int NbRead=0;
				
					static int CompteSample=0;
					CompteSample++;
					int i;
					for(i=0;i<DmaSampleBurstSize;i++)
					{
						NbRead=read(FileInHandle,TabRfSample+i,sizeof(samplerf_t));
						if(NbRead!=sizeof(samplerf_t)) 
						{
							if(loop_mode_flag==1)
							{
								//printf("Looping FileIn\n");
								close(FileInHandle);
								FileInHandle = open(FileName, 'r');
								NbRead=read(FileInHandle,TabRfSample+i,sizeof(samplerf_t));
							}
							else if (FileInHandle != STDIN_FILENO) 
							{
								sleep(1);	
								terminate(0);
							}
						}
						//for(i=0;i<DmaSampleBurstSize;i++)
						{	
							static int amp=32767;
							if(TabRfSample[i].Frequency==0.0)
							{
								amp=0;
								TabRfSample[i].Frequency=00.0;// TODO change that ugly frequency
							}
							else
								amp=32767;
							//TabRfSample[i].WaitForThisSample=((i%2)==0)?50000000L:10000000L;
							//TabRfSample[i].Frequency=((i%2)==0)?144100000.0:144101000.0;
							//printf("Freq =%f Timing=%d %f\n",TabRfSample[i].Frequency,TabRfSample[i].WaitForThisSample,TabRfSample[i].Frequency+GlobalTuningFrequency);
							//TabRfSample[i].Frequency=0;
							FrequencyAmplitudeToRegister(TabRfSample[i].Frequency+GlobalTuningFrequency,amp,last_sample++,30000,TabRfSample[i].WaitForThisSample,SampleRate);
							free_slots--;
							if (last_sample == NUM_SAMPLES)	last_sample = 0;
						}
					}


				}
				// *************************************** MODE RFA : With Amplitude **************************************************
				if(Mode==MODE_RFA)
				{
					int NbRead=0;
				
					static int CompteSample=0;
					CompteSample++;
					int i;
					for(i=0;i<DmaSampleBurstSize;i++)
					{
						NbRead=read(FileInHandle,TabRfSample+i,sizeof(samplerf_t));
						if(NbRead!=sizeof(samplerf_t)) 
						{
							if(loop_mode_flag==1)
							{
								//printf("Looping FileIn\n");
								close(FileInHandle);
								FileInHandle = open(FileName, 'r');
								NbRead=read(FileInHandle,TabRfSample+i,sizeof(samplerf_t));
							}
							else if (FileInHandle != STDIN_FILENO) 
								terminate(0);
						}
						//for(i=0;i<DmaSampleBurstSize;i++)
						{	
							
							
							FrequencyAmplitudeToRegister(GlobalTuningFrequency,TabRfSample[i].Frequency/*This is the amplitude:To be fixed*/,last_sample++,30000,TabRfSample[i].WaitForThisSample,SampleRate);
							free_slots--;
							if (last_sample == NUM_SAMPLES)	last_sample = 0;
						}
					}


				}
				// *************************************** MODE VFO **************************************************
				if(Mode==MODE_VFO)
				{
					
				
					static int CompteSample=0;
					CompteSample++;
					int i;
					for(i=0;i<DmaSampleBurstSize;i++)
					{						
							//To be fine tuned !!!!						
							FrequencyAmplitudeToRegister(GlobalTuningFrequency,32767,last_sample++,10000,1000000/*100us*/,SampleRate);
							free_slots--;
							if (last_sample == NUM_SAMPLES)	last_sample = 0;
						
					}


				}

			}
			
			
		
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			time_difference = gettime_now.tv_nsec - start_time;
			if(time_difference<0) time_difference+=1E9;
			
			

			last_cb = (uint32_t)virtbase + last_sample * sizeof(dma_cb_t) * CBS_SIZE_BY_SAMPLE;
			
		}
				
	

	terminate(0);
	return(0);
}

