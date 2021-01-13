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
#include <ctype.h>
#include <librpitx/librpitx.h>

#define MORSECODES 37

typedef struct morse_code
{
	uint8_t ch;
	const char dits[8];
} Morsecode;

const Morsecode code_table[]  =
{
	{' ', "    "}, // space, 4 dits
	{'0', "-----  "},
	{'1', ".----  "},
	{'2', "..---  "},
	{'3', "...--  "},
	{'4', "....-  "},
	{'5', ".....  "},
	{'6', "-....  "},
	{'7', "--...  "},
	{'8', "---..  "},
	{'9', "----.  "},
	{'A', ".-  "},
	{'B', "-...  "},
	{'C', "-.-.  "},
	{'D', "-..  "},
	{'E', ".  "},
	{'F', "..-.  "},
	{'G', "--.  "},
	{'H', "....  "},
	{'I', "..  "},
	{'J', ".---  "},
	{'K', "-.-  "},
	{'L', ".-..  "},
	{'M', "--  "},
	{'N', "-.  "},
	{'O', "---  "},
	{'P', ".--.  "},
	{'Q', "--.-  "},
	{'R', ".-.  "},
	{'S', "...  "},
	{'T', "-  "},
	{'U', "..-  "},
	{'V', "...-  "},
	{'W', ".--  "},
	{'X', "-..-  "},
	{'Y', "-.--  "},
	{'Z', "--..  "}
};

/*
void SimpleTestOOK(uint64_t Freq)
{

	int SR = 10; //10 HZ
	int FifoSize = 21; //24
	ookburst ook(Freq, SR, 14, FifoSize,100);

	unsigned char TabSymbol[FifoSize] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0};

	ook.SetSymbols(TabSymbol, FifoSize);
}
*/

/**
    Transmits CW OOK data at given rate and frequency.
 */
void Send_CW_OOK(const float freq, const float symbolrate, const char * cw)
{
	//SR = 4, upsample = 200 gives ~5 WPM with dit of 265ms
	//float Freq = 433000000;
	//int FifoSize = 21; //24
	//float symbolrate = 4; //10 HZ
	float upsample = 125.0;
	int FifoSize = strlen((char*)cw) - 1;
	ookburst ook(freq, symbolrate, 14, FifoSize, upsample);

	unsigned char TabSymbol[FifoSize]; // = {0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0};
	for (int i = 0; i <= FifoSize; i++)
	{
		TabSymbol[i] = (cw[i] == '0') ? 0 : 1;
		//printf("%d",TabSymbol[i]);
	}
	ook.SetSymbols(TabSymbol, FifoSize);
}

/**
    Converts morse character to CW OOK binary and sends to transmit.
 */
void morse_to_cw(const char * dits)
{
	char cw[23];// = "mmmmmmmmmmmmmmmmmmmmmmm";
	int dits_len = strlen(dits);
	int a = 0;
	for (int i = 0; i < dits_len; i++)
	{
		if (dits[i] == '.')
		{
			cw[a++] = '1';
			cw[a++] = '0';
		}
		else if (dits[i] == '-')		
		{
			cw[a++] = '1';
			cw[a++] = '1';
			cw[a++] = '1';
			cw[a++] = '0';
		}
		else if (dits[i] == ' ')
		{
			cw[a++] = '0';
		}
	}
	cw[a] = '\0';

	//printf("%s\n", cw);
//	Send_CW_OOK(433000000, cw);
}

void morse_to_cw(const char * dits, char * cw)
{
	//char cw[23];// = "mmmmmmmmmmmmmmmmmmmmmmm";
	int dits_len = strlen(dits);
	int a = 0;
	for (int i = 0; i < dits_len; i++)
	{
		if (dits[i] == '.')
		{
			cw[a++] = '1';
			cw[a++] = '0';
		}
		else if (dits[i] == '-')		
		{
			cw[a++] = '1';
			cw[a++] = '1';
			cw[a++] = '1';
			cw[a++] = '0';
		}
		else if (dits[i] == ' ')
		{
			cw[a++] = '0';
		}
	}
	cw[a] = '\0';

	//printf("%s\n", cw);
	//Send_CW_OOK(cw);
}

/**
    Goes through message converting characters to morse and sends to 
	transmit one at a time.
 */
void text_to_morse(const char * txt)
{
	//const char *test_msg = "TEST MESSAGEZ1";
	int txt_len = strlen(txt);
	for (int i = 0; i < txt_len; i++)
	{
		char tch = toupper(txt[i]);
		for (int j = 0; j < MORSECODES; j++)
		{
			if (code_table[j].ch == tch)
			{
				char * dits = (char*)code_table[j].dits;
				printf("%s", dits);
				morse_to_cw(dits);
			}
		}
	}
}

char * text_to_morse(const char txt)
{
	char tch = toupper(txt);
	for (int j = 0; j < MORSECODES; j++)
	{
		if (code_table[j].ch == tch)
		{
			char * dits = (char*)code_table[j].dits;
			//printf("%s", dits);
			//morse_to_cw(dits);
			return dits;
		}
	}
	return 0;
}

int main(int argc, char * argv[])
{
	const char * msg = "TEST MESSAGEZ1 MSG2 MSG3 MSG0 MSG9";
	float freq = 433000000;
	float wpm = 5;
	//KN4SWZ
	//text_to_morse(test_msg);
	//usage: ./morse freq(Hz) rate(Hz) msg

	if (argc < 4)
	{
		printf("usage: ./morse freq(Hz) rate(dits) MSG(\"quoted\")\n");
		exit(0);
	}

	freq = atof(argv[1]);
	wpm = atof(argv[2]);
	msg = argv[3];
	printf("msg: %s\n", msg);
	char * dits;
	char cw[23];
	float symbol_rate = wpm/1.25;

	for (int i = 0; i < strlen(msg); i++)
	{
		dits = text_to_morse(msg[i]);
		morse_to_cw(dits, cw);
		printf("msg[%02d]: %c\tmorse[%s]\tcw[%s]\n", i, toupper(msg[i]), dits, cw);
		Send_CW_OOK(freq, symbol_rate, cw);
	}
}
