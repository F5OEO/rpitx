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
#include "../librpitx/src/librpitx.h"

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
ngfmdmasync *fsqmod=NULL;
int FifoSize=1000; 
float Frequency=0;
// Global variables used in ISRs
volatile bool proceed = false;

// Define the structure of a varicode table
typedef struct fsq_varicode
{
    uint8_t ch;
    uint8_t var[2];
} Varicode;
 
// The FSQ varicode table, based on the FSQ Varicode V3.0
// document provided by Murray Greenman, ZL1BPU

const Varicode code_table[]  =
{
	{' ', {00, 00}}, // space
	{'!', {11, 30}},
	{'"', {12, 30}},
	{'#', {13, 30}},
	{'$', {14, 30}},
	{'%', {15, 30}},
	{'&', {16, 30}},
	{'\'', {17, 30}},
	{'(', {18, 30}},
	{')', {19, 30}},
	{'*', {20, 30}},
	{'+', {21, 30}},
	{',', {27, 29}},
	{'-', {22, 30}},
	{'.', {27, 00}},
	{'/', {23, 30}},
	{'0', {10, 30}},
	{'1', {01, 30}},
	{'2', {02, 30}},
	{'3', {03, 30}},
	{'4', {04, 30}},
	{'5', {05, 30}},
	{'6', {06, 30}},
	{'7', {07, 30}},
	{'8', {8, 30}},
	{'9', {9, 30}},
	{':', {24, 30}},
	{';', {25, 30}},
	{'<', {26, 30}},
	{'=', {00, 31}},
	{'>', {27, 30}},
	{'?', {28, 29}},
	{'@', {00, 29}},
	{'A', {01, 29}},
	{'B', {02, 29}},
	{'C', {03, 29}},
	{'D', {04, 29}},
	{'E', {05, 29}},
	{'F', {06, 29}},
	{'G', {07, 29}},
	{'H', {8, 29}},
	{'I', {9, 29}},
	{'J', {10, 29}},
	{'K', {11, 29}},
	{'L', {12, 29}},
	{'M', {13, 29}},
	{'N', {14, 29}},
	{'O', {15, 29}},
	{'P', {16, 29}},
	{'Q', {17, 29}},
	{'R', {18, 29}},
	{'S', {19, 29}},
	{'T', {20, 29}},
	{'U', {21, 29}},
	{'V', {22, 29}},
	{'W', {23, 29}},
	{'X', {24, 29}},
	{'Y', {25, 29}},
	{'Z', {26, 29}},
	{'[', {01, 31}},
	{'\\', {02, 31}},
	{']', {03, 31}},
	{'^', {04, 31}},
	{'_', {05, 31}},
	{'`', {9, 31}},
	{'a', {01, 00}},
	{'b', {02, 00}},
	{'c', {03, 00}},
	{'d', {04, 00}},
	{'e', {05, 00}},
	{'f', {06, 00}},
	{'g', {07, 00}},
	{'h', {8, 00}},
	{'i', {9, 00}},
	{'j', {10, 00}},
	{'k', {11, 00}},
	{'l', {12, 00}},
	{'m', {13, 00}},
	{'n', {14, 00}},
	{'o', {15, 00}},
	{'p', {16, 00}},
	{'q', {17, 00}},
	{'r', {18, 00}},
	{'s', {19, 00}},
	{'t', {20, 00}},
	{'u', {21, 00}},
	{'v', {22, 00}},
	{'w', {23, 00}},
	{'x', {24, 00}},
	{'y', {25, 00}},
	{'z', {26, 00}},
	{'{', {06, 31}},
	{'|', {07, 31}},
	{'}', {8, 31}},
	{'~', {00, 30}},
	{127, {28, 31}}, // DEL
	{13,  {28, 00}}, // CR
	{10,  {28, 00}}, // LF
	{0,   {28, 30}}, // IDLE
	{241, {10, 31}}, // plus/minus
	{246, {11, 31}}, // division sign
	{248, {12, 31}}, // degrees sign
	{158, {13, 31}}, // multiply sign
	{156, {14, 31}}, // pound sterling sign
	{8,   {27, 31}}  // BS
};
 
void encode_tone(uint8_t tone);

// Define an upper bound on the number of glyphs.  Defining it this
// way allows adding characters without having to update a hard-coded
// upper bound.
#define NGLYPHS         (sizeof(code_table)/sizeof(code_table[0]))

// Timer interrupt vector.  This toggles the variable we use to gate
// each column of output to ensure accurate timing.  Called whenever
// Timer1 hits the count set below in setup().
/*ISR(TIMER1_COMPA_vect)
{
    proceed = true;
}
*/
// This is the heart of the beacon.  Given a character, it finds the
// appropriate glyph and sets output from the Si5351A to key the
// FSQ signal.



void encode_char(int ch)
{
	uint8_t i, fch, vcode1, vcode2;
	
	for(i = 0; i < NGLYPHS; i++)
	{
		// Check each element of the varicode table to see if we've found the
		// character we're trying to send.
		fch = code_table[i].ch;
		
		if(fch == ch)
		{
			// Found the character, now fetch the varicode chars
			vcode1 = code_table[i].var[0];
			vcode2 = code_table[i].var[1];
			
			// Transmit the appropriate tone per a varicode char
			if(vcode2 == 0)
			{
				// If the 2nd varicode char is a 0 in the table,
				// we are transmitting a lowercase character, and thus
				// only transmit one tone for this character.

				// Generate tone
				encode_tone(vcode1);
			}
			else
			{
				// If the 2nd varicode char is anything other than 0 in
				// the table, then we need to transmit both

				// Generate 1st tone
				encode_tone(vcode1);
				
				// Generate 2nd tone
				encode_tone(vcode2);
			}           
			break; // We've found and transmitted the char,
			// so exit the for loop
		}
	}
}

void encode_tone(uint8_t tone)
{
	cur_tone = ((cur_tone + tone + 1) % 33);
	//printf("Current tone =%d\n",cur_tone);
	//WriteTone(1000 + (cur_tone * TONE_SPACING*0.001),500000L);
	
	int count=0;   
	while(count<1000)
	{
			int Available=fsqmod->GetBufferAvailable();
			if(Available>FifoSize/2)
			{	
					int Index=fsqmod->GetUserMemIndex();			
					for(int j=0;j<Available;j++)
					{
							fsqmod->SetFrequencySample(Index+j,1000 + (cur_tone * TONE_SPACING*0.001));
							count++;
							
					}
									
			}
			else
				usleep(100);	
		
	
	}
			
	//TO DO FREQUENCY PI si5351.set_freq((freq * 100) + (cur_tone * TONE_SPACING), 0, SI5351_CLK0);
}
 
// Loop through the string, transmitting one character at a time.
void encode(char *str)
{
	// Reset the tone to 0 and turn on the output
	cur_tone = 0;
	/*
	si5351.output_enable(SI5351_CLK0, 1);
	digitalWrite(LED_PIN, HIGH);
	*/
	//Serial.println("=======");
	
	// Transmit BOT
	//noInterrupts();
	
	
	encode_char(' '); // Send a space for the dummy character
	//Do wait
	
	// Send another space
	encode_char(' ');
	//Do wait
	
	// Now send LF
	encode_char(10);
	//Do wait
	
	// Now do the rest of the message
	while (*str != '\0')
	{
		encode_char(*str++);
	}
	   
	// Turn off the output
	//si5351.output_enable(SI5351_CLK0, 0);
	//digitalWrite(LED_PIN, LOW);
}

static void init_crc8(void)
{
	int i,j;
	uint8_t crc;

	for(i = 0; i < 256; i++)
	{
		crc = i;
		for(j = 0; j < 8; j++)
		{
			crc = (crc << 1) ^ ((crc & 0x80) ? 0x07 : 0);
		}
		crc8_table[i] = crc & 0xFF;
	}
}

uint8_t crc8(char * text)
{
	uint8_t crc='\0';
	uint8_t ch;
	size_t i;
	for(i = 0; i < strlen(text); i++)
	{
		ch = text[i];
		crc = crc8_table[(crc) ^ ch];
		crc &= 0xFF;
	}

	return crc;
}
 
int main(int argc, char **argv)
{
	char *sText;
	if (argc > 2) 
	{
		sText=(char *)argv[1];
		//FileText = open(argv[1], O_RDONLY);

		
		Frequency = atof(argv[2]);
	}
	else
	{
		printf("usage : pifsq StringToTransmit Frequency(in Hz)\n");
		exit(0);
	}
  
	// Initialize the CRC table
	init_crc8();

	// Generate the CRC for the callsign
	callsign_crc = crc8(callsign);

	
	// We are building a directed message here, but you can do whatever.
	// So apparently, FSQ very specifically needs "  \b  " in
	// directed mode to indicate EOT. A single backspace won't do it.
	sprintf(tx_buffer, "%s:%02x%s%s", callsign, callsign_crc,sText,"  \b  ");
	
	int SR=2000;
	fsqmod = new ngfmdmasync(Frequency,SR,14,FifoSize);

	encode(tx_buffer);
		
  	fsqmod->stop();
	delete(fsqmod);
	
}
 
