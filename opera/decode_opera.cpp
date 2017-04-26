// OPERA_Decode_Test.cpp : Defines the entry point for the console application.
//
// Purpose : to study coding and decoding of OPERA which was developed by EA5HVK.
//
// Usage : "OPERA_Decode_Test [? | d | s | w] 
//          where : s= help, d = debug, s = AA1AA and w = 7L1RLL
//
// Version : 1.0.3, 11/27/2015 change print_char() to print_str()
// Version : 1.0.2, 11/27/2015 bug fix at unpack(), and add 7L1RLL as a 2nd sample.
// Version : 1.0.1, 11/27/2015 Add a function of print_char()
// Version : 1.0.0, 11/25/2015 Initial Release, but under construction on error correction.
//
// Copyright : 7L1RLL(Rick Wakatori) 2015 under Terms of The GNU General
//             Public License.
//
// Environments : Microsoft Visual C++ 2010 Express under Windows 10.
//                Compiler version : 10.0.40219.1 SPIRel
//
// Acknowledgements : 
//  a)EA5HVK(Jose) for work on OPERA
//  b)F4GCB(Patrick)  for PIC program on CRC16 which is a copy into this program.
//  c)PE1NNZ(Guido), for Article titled Opera Protocol Specification.

//#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

short int call_AA1AA[239] = 
{   // callsign = "AA1AA"
    1,0,1,1, 0,1,0,0, 1,0,1,1, 0,0,1,0, // 0xB4, 0xB2
    1,1,0,0, 1,1,0,1, 0,0,1,1, 0,0,1,0, // 0xCD, 0x32
    1,0,1,0, 1,1,0,0, 1,0,1,1, 0,0,1,1, // 0xAC, 0xB3
    0,1,0,0, 1,0,1,0, 1,1,0,0, 1,1,0,0, // 0x4A, 0xCC
    1,1,0,0, 1,1,0,1, 0,0,1,1, 0,0,1,0, // 0xCD, 0x32
    1,0,1,0, 1,0,1,1, 0,0,1,1, 0,1,0,1, // 0xA9, 0x35
    0,1,0,0, 1,1,0,0, 1,1,0,1, 0,1,0,0, // 0x4C, 0xD4
    1,0,1,1, 0,1,0,0, 1,0,1,0, 1,1,0,1, // 0xB4, 0xAD
    0,0,1,1, 0,1,0,0, 1,1,0,1, 0,1,0,0, // 0x34, 0xD4
    1,1,0,0, 1,0,1,0, 1,0,1,1, 0,0,1,1, // 0xCA, 0xB3
    0,0,1,0, 1,0,1,0, 1,1,0,1, 0,1,0,0, // 0x2A, 0xD4
    1,0,1,0, 1,1,0,1, 0,1,0,1, 0,1,0,1, // 0xAD, 0x55
    0,1,0,0, 1,0,1,0, 1,1,0,1, 0,0,1,1, // 0x4A, 0xD3
    0,0,1,1, 0,1,0,1, 0,0,1,0, 1,1,0,0, // 0x35, 0x2C
    1,1,0,1, 0,1,0,0, 1,1,0,0, 1,0,1    // 0xD4, 0xCA 
};

short int call_F5OEO[239] = {1,0,1,1, 0,1,0,1, 0,0,1,0, 1,1,0,1, 0,1,0,1, 0,1,0,1, 0,1,0,0, 1,1,0,1, 0,1,0,0, 1,1,0,0,
1,1,0,0, 1,1,0,0, 1,0,1,0, 1,1,0,1, 0,1,0,1, 0,0,1,0, 1,0,1,0, 1,1,0,1, 0,0,1,1, 0,1,0,0,
1,0,1,0, 1,0,1,1, 0,1,0,1, 0,1,0,1, 0,0,1,0, 1,0,1,0, 1,1,0,1, 0,0,1,1, 0,0,1,1, 0,0,1,0,
1,1,0,0, 1,1,0,1, 0,0,1,0, 1,1,0,1, 0,1,0,1, 0,1,0,0, 1,0,1,1, 0,1,0,0, 1,0,1,1, 0,0,1,1,
0,1,0,1, 0,0,1,1, 0,1,0,1 ,0,1,0,0, 1,0,1,0, 1,0,1,0, 1,1,0,1, 0,1,0,0, 1,1,0,1, 0,1,0,1,
0,0,1,0, 1,0,1,1, 0,0,1,0, 1,1,0,1 ,0,0,1,1, 0,1,0,1, 0,0,1,1, 0,1,0,0, 1,1,0,0, 1,0,1
};



short int call_7L1RLL[239] =
{
	1,0,1,0, 1,1,0,1, 0,0,1,0, 1,1,0,1, // 0xAD, 0x2D
	0,0,1,0, 1,0,1,0, 1,1,0,0, 1,0,1,0, // 0x2A, 0xCA
	1,0,1,0, 1,0,1,0, 1,0,1,1, 0,1,0,0, // 0xAA, 0xB4
	1,0,1,1, 0,0,1,0, 1,0,1,0, 1,0,1,1, // 0xB2, 0xAB
	0,0,1,1, 0,1,0,1, 0,0,1,0, 1,0,1,0, // 0x35, 0x2A
	1,0,1,0, 1,0,1,1, 0,1,0,1, 0,0,1,1, // 0xAB, 0x53
	0,0,1,1, 0,0,1,0, 1,1,0,0, 1,1,0,0, // 0x32, 0xCC
	1,1,0,1, 0,0,1,0, 1,0,1,0, 1,0,1,1, // 0xD2, 0xAB
	0,0,1,1, 0,0,1,0, 1,1,0,0, 1,0,1,1, // 0x32, 0xCB
	0,1,0,0, 1,1,0,0, 1,1,0,1, 0,1,0,1, // 0x4C, 0xD5
	0,1,0,1, 0,0,1,1, 0,1,0,1, 0,0,1,1, // 0x53, 0x53
	0,0,1,0, 1,1,0,0, 1,1,0,0, 1,1,0,1, // 0x2C, 0xCD
	0,1,0,0, 1,1,0,1, 0,1,0,0, 1,0,1,1, // 0x4D, 0x4B
	0,1,0,0, 1,1,0,0, 1,0,1,1, 0,1,0,0, // 0x4C, 0xB4
	1,0,1,0, 1,0,1,0, 1,0,1,0, 1,1,0    // 0xAA, 0xAC
};

short int interleave_target[119] =
{   //after interleave
	1,0,0,1, 1,0,1,1, 0,1,0,0, 1,0,1,1, //0x9B, 0x4B
	1,1,0,1, 1,0,1,0, 0,1,1,1, 0,1,0,1, //0xDA, 0x75
	0,1,0,0, 1,0,1,1, 1,1,1,0, 1,0,0,0, //0x4B, 0xE8
	0,1,0,1, 0,0,0,1, 1,0,0,1, 1,1,0,0, //0x51, 0x9C
	1,0,0,1, 0,0,0,1, 0,1,1,1, 1,0,1,0, //0x91, 0x7A
	1,1,1,1, 0,0,0,1, 1,1,0,0, 0,0,0,0, //0xF1, 0xC0
	0,1,1,1, 0,0,1,0, 1,0,0,0, 1,1,0,1, //0x72, 0x8D
	0,0,0,1, 0,1,1                      //0x16
};

short int before_interleave_target[119] =
{   // before interleave
	1,1,0,1, 0,0,1,0, 0,0,0,0, 0,0,0,1, // 0xD2, 0x01
	1,0,0,1, 1,1,1,0, 0,1,1,0, 1,0,1,1, // 0x9E, 0x6B
	0,1,0,0, 1,1,1,1, 0,0,1,0, 1,0,1,0, // 0x4F, 0x2A
	1,1,0,1, 0,1,0,1, 0,1,1,1, 1,0,0,1, // 0xD5, 0x79
	1,0,1,0, 0,1,0,1, 1,1,1,0, 0,0,0,0, // 0xA5, 0xE0
	0,0,0,0, 1,1,0,0, 1,1,0,0, 0,0,1,1, // 0x0C, 0xC3
	1,1,1,1, 0,0,1,1, 0,1,0,1, 0,1,0,1, // 0xF3, 0x55
	1,1,0,1, 0,0,1                      // 0xD2
};

short int walsh_matrix[8][7] = {
	{0,0,0,0,0,0,0},{1,0,1,0,1,0,1},{0,1,1,0,0,1,1},{1,1,0,0,1,1,0},
	{0,0,0,1,1,1,1},{1,0,1,1,0,1,0},{0,1,1,1,1,0,0},{1,1,0,1,0,0,1}
};

short int pseudo_sequence[51] =
{
	1,1,1,0, 0,0,0,1, 0,1,0,1, 0,1,1,1, // 0xE1, 0x57
	1,1,1,0, 0,1,1,0, 1,1,0,1, 0,0,0,0, // 0xE6, 0xD0
	0,0,0,1, 1,0,0,1, 0,0,0,1, 0,1,0,1, // 0x19, 0x15
	0,1,1                               // 0x60
};

char before_scramble_target[52] =
	"000000000110110001101111000011110001111000001100100";

short int before_WH_target[52] =
{
	1,1,1,0, 0,0,0,1, 0,0,1,1, 1,0,1,1, // 0xE1, 0x3B
	1,0,0,0, 1,0,0,1, 1,1,0,1, 1,1,1,1, // 0x89, 0xDF
	0,0,0,0, 0,1,1,1, 0,0,0,1, 1,0,0,1, // 0x07, 0x19
	1,1,1                               // 0xE0
};

char packed_target[29] ="0000011011000110111100001111"; //0x0606F0F

char call_target[7] = "AA1AA ";

// **** Grobal variables ****
short int symbol[239];           // to be decode
short int interleaved[119];
short int before_interleave[119];
short int error_position[239];   // does not use in V1.0.0
short int before_WH[51];
char before_scramble[51];
char packed[28];
char call[7];
short int DEBUG = 0;

// **** functions ****
void decode_opera(short int symbol[239]);
void manchester_decode(short int symbol[239], short int interleaved[119]);
void de_interleave(short int interleaved[119], short int before_interleave[119]);
void de_walsh_matrix(short int before_interleave[119], short int before_WH[51]);
void de_scramble(short int befor_WH[51], char before_scramble[51]);
void de_crc(char before_scramble[51], char packed[28]);
void generate_crc(char datas[28], char crc[17], int length);
void unpack(char packed[28], char call[7]);
char de_normalizer(int bc, int byte_num);
void print_help();
void print_short_int(const char* caption, short int* code, int length);
void print_str(const char* caption, char* code);
void strcpy_w(char * s1, char * s2, int length);
void strcat_w(char * s1, char * s2, int lenS1, int lenS2);

//**********************************
int main(int argc, char* argv[])
//**********************************
{
	char s1 = 0x00;
	int i;

	printf("argc=%d\n", argc);
	switch(argc)
	{
	case 1 : // Help required
		{
			printf("%s\n","More argument is required !");
			print_help();
			return 0;
		}
	case 2 : // 2 arguments
		{
			s1 = (char) argv[1][0];
			
			if ((s1 == '?') && (argv[1][1] == 0x00))
			{
				printf("%s\n", "Help selected");
				print_help();
				return 0;
			}
			else if ((s1 == 'd' || s1 == 's' || s1 == 'w') && (argv[1][1] == 0x00))
			{
				if (s1 =='d')
				{
					printf("%s\n", "Debug selected.");
				    DEBUG = 1;
					for (i = 0; i < 239; i++)
					    symbol[i] = call_AA1AA[i];
				    decode_opera(symbol);
				    DEBUG = 0;
					return 0;
				}
				else if (s1 == 's')
				{
					printf("%s\n", "Sample selected.");
				    for (i = 0; i < 239; i++)
					    symbol[i] = call_F5OEO[i];
				    decode_opera(symbol);
				    DEBUG = 0;
				    return 0;
				}
				else
				{
					printf("%s\n", "Sample 7L1RLL selected.");
					DEBUG = 0;
				    for (i = 0; i < 239; i++)
					    symbol[i] = call_7L1RLL[i];
				    decode_opera(symbol);
				    DEBUG = 0;
				    return 0;
				}
    		}
		}
	default  : // 3 arguments
		{
			printf("%s\n", "Too many arguments.");
			print_help();
			return 0;
		}
	}
} // end of _tmain()

//**************************************************
void decode_opera(short int * symbol)
//**************************************************
{
    print_short_int("symbol given            =", symbol, 239);
	manchester_decode(symbol, interleaved);
	print_short_int("de_manchester code      =", interleaved, 119);
	if (DEBUG)
	   print_short_int("de_manchester_target    =", interleave_target, 119);
	de_interleave(interleaved, before_interleave);
	print_short_int("de_interleave           =", before_interleave, 119);
	if (DEBUG)
		print_short_int("de_interleave_target   =", before_interleave_target, 119);
    de_walsh_matrix(before_interleave, before_WH);
	print_short_int("de_Walsh-Hamadard       =", before_WH, 51);
	if (DEBUG)
		print_short_int("de_WH_target           =", before_WH_target, 51);
	de_scramble(before_WH, before_scramble);
	print_str("de_scramble                  = ", before_scramble);
	if (DEBUG)
		print_str("de_scramble_target          = ", before_scramble_target);
	de_crc(before_scramble, packed);
	print_str("de_CRC                          = ", packed);
	if (DEBUG)
		print_str("de_CRC_target    = ", packed_target);
	unpack(packed, call);
	printf("unpached call             = %s\n", call);
	if (DEBUG)
		printf("call_target      = %s\n", call_target);
} // end of decode_opera()

//***********************************************************************
void manchester_decode(short int* symbol, short int* symbol_interleaving)
//***********************************************************************
{   
	int i   = 0;
	int idx = 0;  // delete start 2 bit

	while (idx < 238)
	{
		if ((symbol[idx + 1] == 1) && (symbol[idx + 2] == 0))
		{
			symbol_interleaving[i] = (short) 0;
		}
		else if ((symbol[idx + 1] == 0) && (symbol[idx + 2] == 1))
		{
			symbol_interleaving[i] = (short) 1;
		}
		else
		{
			error_position[idx] = (short) 1;
		}
		i++; idx += 2;	
	}
} // end of manchester_decode()

//************************************************************************
void de_interleave(short int* interleaved, short int* before_interleave)
//************************************************************************
{
	int i =0, idx = 0, j = 0;

	for (i = 0; i < 7; i++)
	{
		for (j = i;  j < 119;  j += 7)
		{
			before_interleave[j] = interleaved[idx];
			idx++;
		}
	}
} // end of de_interleave

//*********************************************************************
void de_walsh_matrix(short int* vector_to_tx, short int* symbol_coding)
//*********************************************************************
{   // 119 bit to 51 bit
	int idx = 0, i, j, k, data = 0;
	short int temp[7];

	for (i = 0; i < 119; i += 7)
	{ 
		for(j = 0; j < 7; j++) temp[j] = vector_to_tx[i + j];
		temp[7]=0x00;
		   
		// search the value  for match
		data = 0;
		for (k = 0; k < 7; k++)
		{ 
			if (k < 6)
			{
			  data = data + (int) pow((double)(temp[k]*2), (6 - k));
			}
			else if (k == 6)
			{
				if (temp[6] == 1) data = data + 1;
			}	
		}	
		if (data == 0) // 0000000 = 0
		{
		     symbol_coding[idx + 0] = 0; 
			 symbol_coding[idx + 1] = 0;
		     symbol_coding[idx + 2] = 0;
		}
		else if (data == 85)  // 1010101 = 2^6 + 2^4 + 2^2 + 1 = 85
		{
			 symbol_coding[idx + 0] = 0;
			 symbol_coding[idx + 1] = 0;
			 symbol_coding[idx + 2] = 1;
		}  
		else if (data == 51)  // 0110011 = 2^5 + 2^4 + 2 + 1 = 51
		{
			 symbol_coding[idx + 0] = 0;
			 symbol_coding[idx + 1] = 1;
			 symbol_coding[idx + 2] = 0;
		}
		else if (data == 102) // 1100110 = 2^6 + 2^5 + 2^2 + 2 = 102
		{
			 symbol_coding[idx + 0] = 0;
			 symbol_coding[idx + 1] = 1;
			 symbol_coding[idx + 2] = 1;
		}
		else if (data == 15)  // 0001111 = 2^3 + 2^2 + 2 + 1 = 15 
		{
			 symbol_coding[idx + 0] = 1;
			 symbol_coding[idx + 1] = 0;
			 symbol_coding[idx + 2] = 0;
		}
		else if (data == 90) // 1011010 = 2^6 + 2^4 + 2^3 + 2 = 90
		{
			 symbol_coding[idx + 0] = 1;
			 symbol_coding[idx + 1] = 0;
			 symbol_coding[idx + 2] = 1;
		}
		else if (data == 60)  //0111100 = 2^5 + 2^4 + 2^3 + 2^2 = 60
		{
			 symbol_coding[idx + 0] = 1;
			 symbol_coding[idx + 1] = 1;
			 symbol_coding[idx + 2] = 0;
		}
		else if (data == 105)  // 1101001 = 2^6 + 2^5 + 2^3 + 1 = 105
		{
			 symbol_coding[idx + 0] = 1;
			 symbol_coding[idx + 1] = 1;
			 symbol_coding[idx + 2] = 1;
		}
		else printf("xxxx");

		idx +=3;

	}
} // enf of de-WH

//************************************************************
void de_scramble(short int * vector_to_tx, char * vector)
//************************************************************
{   // | 51 bit | to | 51 bit | 
	short int vector_temp[51];
	int i;

	// convert binary to ASCII
	for (i = 0; i < 51; i++)
	{
		vector_temp[i] = vector_to_tx[i] ^ pseudo_sequence[i];
		vector[i] = (char) vector_temp[i] + 0x30;
	}
} //end of de_scramble()

//*************************************************
void de_crc(char * vector, char * packed)
//*************************************************
{  // 51 bits to 28bits

	char crc1[17], crc1a[17], crc2[4], crc2a[4];
	int i, crc_ok;
	char temp[52] = {0};

	// extract packed from received data
	for (i = 0; i < 28; i++)      //4..31 for packed
		packed[i] = temp[i] = vector[i + 4];
	packed[28] = temp[28] = 0x00;

	if (DEBUG)
		print_str("temp in de_crc   = ", temp);
    if (DEBUG)
		print_str("packed in de_crc = ", packed);

	// extract crc1 from received data
	for (i = 0; i < 16; i++)      //32..47 for crc1
		crc1[i] = vector[i + 32];
	crc1[16] = 0x00;

	if (DEBUG)
		print_str("crc1 exracted from received data  = ", crc1);

	//crc2 extracted  from received data
	for (i = 0; i < 3; i++)       //48..50 for crc2
		crc2[i] = vector[i + 48];
	crc2[3] = 0x00;

	if (DEBUG)
		print_str("crc2 extracted from received data = ", crc2);
	
	generate_crc(temp, crc1a, 16);
	if (DEBUG)
		print_str("temp before crc add = ", temp);
	if (DEBUG)
		print_str("crc1a calcurated = ", crc1a);
	strcat_w(temp, crc1a, 28, 16);       // 28 + 16 = 44

	generate_crc(temp, crc2a, 3);
	strcat_w(temp, crc2a, 44, 3);        // 44 +  3 = 47
	
	// verify crc1 and crc2
	crc_ok = 1; 
	for (i = 0; i < 16; i++)
		if (crc1[i] != crc1a[i]) crc_ok = 0;
	for (i = 0; i < 3; i++)
		if (crc2[i] != crc2a[i]) crc_ok = 0;
	if (crc_ok)
	     printf("CRC : OK\n");
	else
	{
		printf("CRC : No good\n");
		print_str("crc1a calcurated = ", crc1a);
	    print_str("crc1_received    = ", crc1);
	    print_str("crc2a calcurated = ", crc2a);
	    print_str("crc2a_received   = ", crc2);
	}
	
} //end of de_crc()

//************************************************************************
void generate_crc(char * datas, char * crc, int length)
//************************************************************************
{   // 32 + 16(length) = 48 or 48 + 3(length) = 51
	// CRC16-IBM : Polynominal = X16+X15+X2+1 = 1000 0000 0000 0101
	// This function is a copy of JUMA TX136/500 control program 
	// whitch was written by F4GCB (Patrick). Thanks Patrick.

	int i, j, k, len;
	char buffer[52];
	short int wcrc[17] = {0}, byte1 = 0, byte2 = 0;

	len = strlen(datas);
	for (i = 0; i < len; i++)
		buffer[i] = datas[i]; 
	buffer[len] = 0x00;
	if (DEBUG)
		print_str("input datas in generate_crc =", buffer);

	for (i = 0; i < len; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (j > 0)	buffer[i] = buffer[i] >>  1; 
			byte1 = buffer[i] & 0x01;
			byte2 = byte1 ^ wcrc[0];     // XOR with X16
			wcrc[0] = byte2 ^ wcrc[1];   // XOR with X15
			for (k = 1; k < 13; k++)
				wcrc[k] = wcrc[k + 1];
			wcrc[13] = byte2 ^ wcrc[14]; // XOR with X2
			wcrc[14] = wcrc[15];         //
			wcrc[15] = byte2;            //      
		}
	}

	// if msb byte crc = 0 then value at 27
	byte2 = 0;
	for (i = 0; i < 8; i++)
		byte2 = byte2 + (int)(wcrc[i] * pow (2.0, i));
	if (byte2 == 0) byte2 = 27;    // 0x1B = 0b0001 1011

	// if lsb byte crc = 0 then value at 43
	byte1 = 0;
	for (i = 8; i < 16; i++)
		byte1 = byte1 + (int)(wcrc[i] * pow(2.0, i - 8));
	if (byte1 == 0) byte1 = 43;   // 0x2B = 0b0010 1011

	if (DEBUG)
		printf("byte1 before replace =%2x, byte2 =%2x\n", byte1, byte2);

	// merge crc into a bit string
	for (i = 0; i < 8; i++)
	{
		if (i > 0) byte2 = byte2 >> 1;
		wcrc[7 - i] = byte2 & 0x01;     // (binary)
		if (i > 0) byte1 = byte1 >> 1;
		wcrc[15 - i] = byte1 & 0x01;    // (binary)
	}
	if (length > 16) length = 16;
	for (i = 16 - length; i < 16; i++)
		crc[i - (16 - length)] = wcrc[i] + 0x30; 
	crc[length]= 0x00;
	if (DEBUG)
		print_str("crc =", crc);
} // end of generate_crc()

//*************************************
void unpack(char * packed, char * call)
//*************************************
{    // 28 bits to 48 bits
	int i;
	int temp;
	unsigned long code_sum = 0, remains = 0;

	if (DEBUG)
		print_str("packed in unpack = ", packed);

	// separate a string to coded callsign
	
	for (i = 0; i < 28; i++)
		code_sum = code_sum + (unsigned long)(packed[27 - i] - '0') * pow(2.0, i);

	// de_normalizer of callsign
	remains = 36*10*27*27*27;
    if (DEBUG)
	{
		printf(" 0 : code_sum    = %9Lu\n", code_sum);
	    printf(" 0 : remains     = %9Lu\n", remains);
	}

	if (code_sum > remains) 
	{
		temp = code_sum / remains;
		code_sum %= remains;	
	} 
	else temp = 0;
	if (DEBUG)
		printf(" 0 : temp        = %9Lu\n", temp);
	call[0] = de_normalizer(temp, 0);

	remains = 10*27*27*27;
	if (DEBUG)
	{
		printf(" 1 : code_sum    = %9Lu\n", code_sum);
	    printf(" 1 : remains     = %9Lu\n", remains);
	}
	if (code_sum >= remains) 
	{
        temp = code_sum / remains;
		code_sum %= remains;
	}
	else temp = 0;
	if (DEBUG)
		printf(" 1 : temp        = %9Lu\n", temp);
	call[1] = de_normalizer(temp, 1);
	
	remains = 27*27*27;
	if (DEBUG)
	{
		printf(" 2 : code_sum    = %9Lu\n", code_sum);
	    printf(" 2 : remains     = %9Lu\n", remains);
	}
	if (code_sum >= remains)
	{
		temp = code_sum / remains;
		code_sum %= remains;
	}
	else temp = 0;
	if (DEBUG)
		printf(" 2 : temp        = %9Lu\n", temp);
	call[2] = de_normalizer(temp, 2);
    
	remains = 27*27;
	if (DEBUG)
	{
		printf(" 3 : code_sum    = %9Lu\n", code_sum);
	    printf(" 3 : remains     = %9Lu\n", remains);
	}
	if (code_sum >=  remains) 
	{
		temp = code_sum / remains;
		code_sum %= remains;
	}
	else temp = 0;
	if (DEBUG)
		printf(" 3 : temp        = %9Lu\n", temp);
	call[3] = de_normalizer(temp, 3);

	remains = 27;
	if (DEBUG)
	{
	   printf(" 4 : code_sum    = %9Lu\n", code_sum);
       printf(" 4 : remains     = %9Lu\n", remains);
	}
	if (code_sum >= remains) 
	{
		temp = code_sum / remains;
		code_sum %= remains;
	}
	else temp = 0;
	if (DEBUG)
		printf(" 4 : temp        = %9Lu\n", temp);
	call[4] = de_normalizer(temp, 4);
	
	remains = 1;
	if (DEBUG)
	{
	   printf(" 5 : code_sum    = %9Lu\n", code_sum);
	   printf(" 5 : remains     = %9Lu\n", remains);
	}
	if (code_sum >= remains) 
	{
		temp = code_sum;
		//code_sum %= remains;
	}
	else temp = 0;
	if (DEBUG)
		printf(" 5 : temp        = %9Lu\n", temp);
	call[5] = de_normalizer(temp, 5);
	
	call[6] = 0x00;
} // end of unpack

//********************************
char de_normalizer(int bc, int n)
//********************************
{
	char cc = 0;

	if (DEBUG)
		printf(" %u : input  of de_normalizer, bc = %9Lu\n", n, bc);
	switch (n)  
	{
	case 0 :
		{
			if (bc == 0) cc = ' ';
			else if (bc >=  1 && bc <= 26) cc = bc - 1 +'A';
			else if (bc >= 27 && bc <= 37) cc = bc - 27 + '0';
			break;
		}
	case 1 :
		{
		    if (bc >= 0 && bc <= 25)       cc = bc + 'A';
			else if (bc >= 26 && bc <= 36) cc = bc - 26 + '0';
		    break;
		}
	case 2 :
		{
			if (bc >= 0 && bc <= 9)        cc = bc + '0'; 
		    break;
		}
	case 3: case 4: case 5:
		{
	       if      (bc ==  0)             cc = ' ';
	       else if (bc >=  1 && bc <= 26) cc = bc - 1 + 'A';
		   break;
		}
	default : break;
	}
    if (DEBUG)
		printf(" %u : output of de_normalizer, cc = %c\n", n, cc);
   return (cc);
} // end of de _normlizer

//********************************************************************
void print_short_int(const char *caption, short int *code, int length)
//********************************************************************
{   // This is a service function for debugging
	int i = 0;

	printf("%s\n", caption);
	for (i = 0; i < length; i++)
	{
		printf("%u", code[i]);
		if (((i + 1) %  4) == 0) printf(" ");
		if (((i + 1) % 40) == 0) printf("\n");
	}
	printf("\n");
} // end fo print_short_int

//********************************************************************
void print_short_char(const char * caption, char * code, int length)
//********************************************************************
{   // This is a service function for debugging
	int i = 0;

	printf("%s\n", caption);
	for (i = 0; i < length; i++)
	{ 
		printf("%c", code[i]);
		if (((i + 1) %  4) == 0) printf(" ");
		if (((i + 1) % 40) == 0) printf("\n");
	}
	printf("\n");
} // end fo print_short_char

//********************************************************************
void print_str(const char * caption, char * code)
//********************************************************************
{   // This is a service function for debugging
	int i = 0;

	printf("%s\n", caption);
	for (i = 0; i < strlen(code); i++)
	{ 
		printf("%c", code[i]);
		if (((i + 1) %  4) == 0) printf(" ");
		if (((i + 1) % 40) == 0) printf("\n");
	}
	printf("\n");
} // end fo print_char
//*******************************************************
void strcpy_w(char * s1, char * s2, int length)
//*******************************************************
{   
	int i;

	for (i = 0; i < length; i++)
		s1[i] = s2[i];
	s1[length] = 0x00;
} // end of strcpy_w

//****************************************************************
void strcat_w(char * s1, char * s2, int lenS1, int lenS2)
//****************************************************************
{   
	int i;
    
    for (i = 0; i < lenS2; i++)
		s1[i + lenS1] = s2[i];
	s1[lenS1 + lenS2]= 0x00;

} // end of strcat_w()

//***************
void print_help()
//***************
{
	printf("%s\n","Usage   : OPERA_Decode_Test [ ? | d | s | w]");
	printf("%s\n"," Help   : OPERA_Decode_Test ?");
	printf("%s\n"," Debug  : OPERA_Decode_Test d");
	printf("%s\n"," AA1AA  : OPERA_Decode_Test s");
	printf("%s\n"," 7L1RLL : OPERA_Decode_Test w");
    printf("%s\n"," Sample callsign is \"AA1AA\". ");
} // end of help

//************** End of Program **************************************
