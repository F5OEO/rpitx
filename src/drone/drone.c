//
// Simple FSQ beacon for Arduino, with the Etherkit Si5351A Breakout
// Board, by Jason Milldrum NT7S.
// 
// Original code based on Feld Hell beacon for Arduino by Mark 
// Vandewettering K6HX, adapted for the Si5351A by Robert 
// Liesenfeld AK6L <ak6l@ak6l.org>.  Timer setup
// code by Thomas Knutsen LA3PNA.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
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

#define TONE_SPACING            8789           // ~8.7890625 Hz
#define BAUD_2                  7812          // CTC value for 2 baud
#define BAUD_3                  5208          // CTC value for 3 baud
#define BAUD_4_5                3472          // CTC value for 4.5 baud
#define BAUD_6                  2604          // CTC value for 6 baud

#define LED_PIN                 13

#define bool char
#define false 0
#define true 1

// Global variables

unsigned long freq = 0;//7105350;   // Base freq is 1350 Hz higher than dial freq in USB
uint8_t cur_tone = 0;
static uint8_t crc8_table[256];
char callsign[10] = "F5OEO";
char tx_buffer[40];
uint8_t callsign_crc;
int FileText;
int FileFreqTiming;
// Global variables used in ISRs
volatile bool proceed = false;


void WriteTone(double Frequency,uint32_t Timing)
{
	typedef struct {
		double Frequency;
		uint32_t WaitForThisSample;
	} samplerf_t;
	samplerf_t RfSample;
	
	RfSample.Frequency=Frequency;
	RfSample.WaitForThisSample=Timing; //en 100 de nanosecond
	//printf("Freq =%f Timing=%d\n",RfSample.Frequency,RfSample.WaitForThisSample);
	if (write(FileFreqTiming, &RfSample,sizeof(samplerf_t)) != sizeof(samplerf_t)) {
		fprintf(stderr, "Unable to write sample\n");
	}

}


 
int main(int argc, char **argv)
{
	char *sText;
	if (argc > 2) 
	{
		sText=(char *)argv[1];
		//FileText = open(argv[1], O_RDONLY);

		char *sFileFreqTiming=(char *)argv[2];
		FileFreqTiming = open(argv[2], O_WRONLY|O_CREAT);
	}
	else
	{
		printf("usage : pidrone StringToTransmit file.ft\n");
		exit(0);
	}
  
	//100KHZ
    int Freq_span = 100000;
    int NbStep = 2000;
    int i;
	for(i=1;i<NbStep;i++)
	{
		WriteTone(Freq_span*i/NbStep-Freq_span/2,1000e6/NbStep);
	}
    WriteTone(00000,250e6);
    for (i=0;i<strlen(sText);i++)
    {
        int bit;
        WriteTone(25000,1250e3);//Start bit
        for(bit=0;bit<8;bit++)
        {
            if((sText[i]&(1<<(7-bit)))>0) 
            {
                WriteTone(25000,1250e3);
                printf("1");
            }
            else
            {
                WriteTone(-25000,1250e3);
                printf("0");
            }
            
        }
        WriteTone(25000,1250e3);//Stop bit
        printf("\n");
    }

     WriteTone(00000,250e6);
	close(FileFreqTiming);
}
 
