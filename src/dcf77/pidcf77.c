
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

// Get main process from arduino project : CodingGhost/DCF77-Transmitter
// Generator for 77,5 kHz (DCF-77) by Jonas woerner (c)
// Thanks to Jonas
// Evariste F5OEO 2015

//!!!!ONLY FOR TESTING PURPOSES!
#define byte char
#define false 0
#define true 1

byte dcfBits[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 };

#define anzahlMinutenBits  7
#define anzahlStundenBits  6
#define offsetMinutenBits  21
#define offsetStundenBits  29
byte MinutenBits[anzahlMinutenBits] = { 0 };
byte StundenBits[anzahlStundenBits] = { 0 };
int parity = 0;
int FileFreqTiming;

void modulate(byte b);
void playtone(double Amplitude,uint32_t Timing);

void loop()
{
	int i;
	for (i = 0; i<58; ++i)
	{
		modulate(dcfBits[i]);
	}

}

void DCF_BITS(int Minuten, int Stunden)
{
	int i;	  
	//MINUTE

	if (Minuten > 39)
	{
		MinutenBits[6] = 1;
		Minuten -= 40;
	}
	if (Minuten > 19)
	{
		MinutenBits[5] = 1;
		Minuten -= 20;
	}
	if (Minuten > 9)
	{
		MinutenBits[4] = 1;
		Minuten -= 10;
	}

	for ( i = 0; i < 4; ++i)
	{
		//MinutenBits[i] = (Minuten & (1 << i)) ? true : false;

		if ((Minuten & (1 << i)) > 0)
		{
			MinutenBits[i] = true;
		}
		else
		{
			MinutenBits[i] = false;
		}
	}
	
	for ( i = 0; i < anzahlMinutenBits; ++i)
	{
		dcfBits[offsetMinutenBits + i] = MinutenBits[i];
	}
	
	//Stunde
	if (Stunden > 19)
	{
		StundenBits[5] = 1;
		Stunden -= 20;
	}
	if (Stunden > 9)
	{
		StundenBits[4] = 1;
		Stunden -= 10;
	}
	 
	for ( i = 0; i < 4; ++i)
	{
		//MinutenBits[i] = (Minuten & (1 << i)) ? true : false;

		if ((Stunden & (1 << i)) > 0)
		{
			StundenBits[i] = true;
		}
		else
		{
			StundenBits[i] = false;
		}
	}

	for ( i = 0; i < anzahlStundenBits; ++i)
	{
		dcfBits[offsetStundenBits + i] = StundenBits[i];
	}

	/*for (int n = 0; n < 6; ++n)
	{

	}*/
	////////////////////////////
	{
		parity += dcfBits[21];
		parity += dcfBits[22];
		parity += dcfBits[23];
		parity += dcfBits[24];
		parity += dcfBits[25];
		parity += dcfBits[26];
		parity += dcfBits[27];
		
		//dcfBits[28] = parity % 2; ???
		if (parity % 2 == 0)
		{
			dcfBits[28] = 0;
		}
		else
		{
			dcfBits[28] = 1;
		}
		parity = 0;
	}
	////////////////////////////
	{
		parity += dcfBits[29];
		parity += dcfBits[30];
		parity += dcfBits[31];
		parity += dcfBits[32];
		parity += dcfBits[33];
		parity += dcfBits[34];
		if (parity % 2 == 0)
		{
			dcfBits[35] = 0;
		}
		else
		{
			dcfBits[35] = 1;
		}
		parity = 0;
	}
	/////////////////////////////
	{
		parity += dcfBits[36];
		parity += dcfBits[37];
		parity += dcfBits[38];
		parity += dcfBits[39];
		parity += dcfBits[40];
		parity += dcfBits[41];
		parity += dcfBits[42];
		parity += dcfBits[43];
		parity += dcfBits[44];
		parity += dcfBits[45];
		parity += dcfBits[46];
		parity += dcfBits[47];
		parity += dcfBits[48];
		parity += dcfBits[49];
		parity += dcfBits[50];
		parity += dcfBits[51];
		parity += dcfBits[52];
		parity += dcfBits[53];
		parity += dcfBits[54];
		parity += dcfBits[55];
		parity += dcfBits[56];
		parity += dcfBits[57];
		if (parity % 2 == 0)
		{
			dcfBits[58] = 0;
		}
		else
		{
			dcfBits[58] = 1;
		}
		parity = 0;
	}
}

void modulate(byte b)
{
	if (b == 0)
	{
		playtone(32667/8,100e6);
		playtone(32767,900e6);
	}
	else
	{
		playtone(32767/8,200e6);
		playtone(32767,800e6);
	}
}

void playtone(double Amplitude,uint32_t Timing)
{
	typedef struct {
		double Amplitude;
		uint32_t WaitForThisSample;
	} samplerf_t;
	samplerf_t RfSample;
	
	RfSample.Amplitude=Amplitude;
	RfSample.WaitForThisSample=Timing; //en 100 de nanosecond
	printf("%f %d\n",Amplitude,Timing);
	if (write(FileFreqTiming, &RfSample, sizeof(samplerf_t)) != sizeof(samplerf_t)) {
		fprintf(stderr, "Unable to write sample\n");
	}
}

int main(int argc, char **argv)
{
	if (argc > 1) 
	{
		char *sFileFreqTiming=(char *)argv[1];
		FileFreqTiming = open(argv[1], O_WRONLY|O_CREAT, 0644);
		
		DCF_BITS(7,59);
		loop();
		playtone(0,1000e6);//last second
		close(FileFreqTiming);
	}
	else
	{
		printf("usage : pidfc77  dcfpatern.bin\n");
		exit(0);
	}
		
	return 0;
}
	



