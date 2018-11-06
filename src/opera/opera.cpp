//******************************************************************************
// OPERA_Coding_Test.cpp : Defines the entry point for the console application.
//
// Purpose : Algorithm testing by a laptop computer before implementation  
// into PIC micro processor.
//
// Usage: "OPERA_Coding_Test [? | d | s] ["callsign"]";
//        where : s = help, d = debug and s = sample";
//
// Version 1.0.4, 2015/11/29: Modified print format
// Version 1.0.3, 2015/11/13: Delete an additional start bit for decoder
// Version 1.0.2, 2015/11/11: Add Visual C++ directives
// Version 1.0.1, 2015/11/10: Changed help message
// Version 1.0.0, 2015/11/07: Initial Release
//
// Copyright(C) 2015 F4GCB
// Partial copyright (C)2015 7L1RLL
// Partial copyright (C)2017 F5OEO Add output format to Rpitx RFA Mode
//

// Acknowledgement :
// 1)Portion of this OPERA is derived from the work of EA5HVK.
// 2)All functions of this program are a copy of JUMA-TX500/136 
// Transmitter Controller which written by F4GCB.
//******************************************************************************

//#include "stdafx.h"
//#include <locale.h>
//#include <tchar.h>
#include "stdio.h"
#include "cstring"
#include <stdlib.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include "stdint.h"
#include "math.h"
#include <signal.h>
 #include <unistd.h>
#include "../librpitx/src/librpitx.h"
bool running=true;
//#define  __VCpp__  TRUE

// Grobal Variables
float Frequency=0;

static const short int pseudo_sequence[51] = {
	1,1,1,0,0,0,0,1,0,1, 0,1,0,1,1,1,1,1,1,0, 0,1,1,0,1,1,0,1,0,0, 0,0,0,0,0,1,1,0,0,1,
	0,0,0,1,0,1,0,1,0,1, 1
};
static short int walsh_matrix[8][7] = {
	{0,0,0,0,0,0,0},{1,0,1,0,1,0,1},{0,1,1,0,0,1,1},{1,1,0,0,1,1,0},
	{0,0,0,1,1,1,1},{1,0,1,1,0,1,0},{0,1,1,1,1,0,0},{1,1,0,1,0,0,1}
};
static short int symbol[239];
char call[7], call_coded[45], vector[52];
short int vector_to_tx[51];
short int symbol_interleaving[119], symbol_coding[119];
short int DEBUG = 0;
const char sampleCall[7] = "AA1AA";

// Declaration of functions

void genn_opera(float mode);
void generate_call(char call[7], char call_coded[45]);
void add_crc16(char call_coded[45], char vector[52]);
void scramble(char vector[52], short int symbol_coding[119]);
void Walsh_Hammered_code(short int symbol_coding[119], short int vector_to_tx[51]);
void interleave(short int vector_to_tx[51], short int symbol_interleaving[119]);
void ManchesterEncode(short int symbol_interleaving[119], short int symbol[239]);
char chr_norm_opera(char bc);

void print_short_int(const char caption[], short int code[239], int length);
void print_str(const char caption[250], char code[52]);
void strcpy_w(char s1[52], char s2[52], int length);
void strcat_w(char s1[52], char s2[52], int lenS1, int lenS2);
void encodepitx(short int *code, int length,float Nop);

#ifdef __VCpp__
//**********************************
// main forVisual C++
int _tmain(int argc, _TCHAR* argv[])
//**********************************
{
	_tsetlocale(LC_ALL, _T(""));  //Change Unicode to OS-Default locale
#else
	int main(int argc, char* argv[])
//**********************************
	{
#endif
	int i = 0;
    //char s1 = 0x00;
	char s2[7] = "";

	
	switch (argc)
	{
	   case 1 : // Help required
       case 2 : // Help required 
         case 3 : // Help required 
	   {
		 printf("Usage : %s CALLSIGN OperaMode[0.5,1,2,4,8} \n", argv[0]);
	
		 return 0;
	   }
	   case 4:  // 3 arguments
	   {		 
		 //s1 = (char)argv[1][0]; 
		         
			// range check
			if (!((argv[1][0] >= '0' && argv[1][0] <= '9') || (argv[1][0] >= 'A' && argv[1][0] <= 'Z') ||
				  (argv[1][0] >= 'a' && argv[1][0] <= 'z')))
	        {
		        printf("%s\n","Callsign must be began with an alphan/numeric character");
		        
		        return 0;
	        }
	        DEBUG = 0; i = 0;
	        while (argv[1][i] != 0 && i < 7)
	        {
		        call[i] = argv[1][i]; call[++i] = 0x00;
	        }
            float Mode=atof(argv[2]);

            Frequency=atof(argv[3]);
                        
			genn_opera(Mode);
			return 0;
	    
	   }
	   
	   default:
	   {
    	   printf("Usage : %s CALLSIGN OperaMode[0.5,1,2,4,8} file.rfa \n", argv[0]);
		   break;
	   }
  } // end of switch argc
  return 0;
} // end of _tmain

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating %x\n",num);
   
}

//*******************
void genn_opera(float mode)
//*******************
{
   printf("\nGenerate Op%.1f Callsign = %s\n",mode,call);

   generate_call(call, call_coded);
   if (DEBUG) 
	  print_str("call_coded =", call_coded);
 	
   add_crc16(call_coded, vector);

   if (DEBUG)
	  print_str("crc16 vector =", vector);
	   
   scramble(vector, vector_to_tx);
   if (DEBUG) 
	  print_short_int("vector_to_tx =", vector_to_tx, 44);
	   
   Walsh_Hammered_code(vector_to_tx, symbol_coding);
   if (DEBUG)
	  print_short_int("symbol_coding =", symbol_coding, 119);
	   
   interleave(symbol_coding, symbol_interleaving);
   if (DEBUG) 
	  print_short_int("symbol_interleaving =", symbol_interleaving, 119);
   
   ManchesterEncode(symbol_interleaving, symbol);
   
   for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

   encodepitx(symbol,239,mode);

} // genn_opera

//****************************************************
// Normalize characters space S..Z 0..9 in order 0..36
char chr_norm_opera(char bc)
//****************************************************
{
	char cc = 0;

	if (bc >= '0' && bc <= '9') cc = bc - '0' + 27;
	if (bc >= 'A' && bc <= 'Z') cc = bc - 'A' + 1;
	if (bc >= 'a' && bc <= 'z') cc = bc - 'a' + 1;
	if (bc == ' ') cc = 0;

	return (cc);
} // enf of chr_norm_opera

//**********************************************
void generate_call(char *call, char *call_coded)
//**********************************************
{
	int i;
	unsigned long code_sum;

	//the thired character must always be a number
	if (chr_norm_opera(call[2]) < 27)
	{
		for (i=5; i> 0; i--) call[i] = call[i-1];
		call[0]=' ';
	}

	// the call must always have 6 characters
	for (i=strlen(call); i < 6; i++)
		call[i] = ' ';
	call[6] = 0x00;

	if (DEBUG) printf("NormalizedCall=%s\n", call);
	code_sum = chr_norm_opera(call[0]);
	code_sum = code_sum * 36 + chr_norm_opera(call[1]) - 1;
	code_sum = code_sum * 10 + chr_norm_opera(call[2]) - 27;
	code_sum = code_sum * 27 + chr_norm_opera(call[3]);
	code_sum = code_sum * 27 + chr_norm_opera(call[4]);
	code_sum = code_sum * 27 + chr_norm_opera(call[5]);
   
	if (DEBUG) printf("code_sum=%lu\n", code_sum);

	// merge coded callsign ino a string
	call_coded[28] = 0x00;
	call_coded[27] = (short int) ((code_sum & 0x01) + 0x30);
	for (i = 26; i >= 0; i--)
	{
		code_sum = code_sum >> 1;
		call_coded[i] = (short int)((code_sum & 0x01) + 0x30);
	}
} // end of pack_callsign

//***************************************************
void generate_crc(char *datas, char *crc, int length)
//***************************************************
{
	unsigned int i, j, k;
	char buffer[52]; //strlen(datas)];
	short int wcrc[16] = {0}, byte1 = 0, byte2 = 0;

#ifdef __VCpp__
	strcpy_s(buffer, 52,  datas);// strcpy(buffer, datas);
#else
	strcpy(buffer, datas);// strcpy(buffer, datas);
#endif
	if (DEBUG) 
		print_str("buffer_crc = ", buffer);

	for (i = 0; i < strlen(datas); i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (j > 0) buffer[i] = buffer[i] >> 1;
			byte1 = buffer[i] & 0x01;
			byte2 = byte1 ^ wcrc[0];
			wcrc[0] = byte2 ^ wcrc[1];
			for (k = 1; k < 13; k++)
				wcrc[k] = wcrc[k+1];
			wcrc[13] = byte2 ^ wcrc[14];
			wcrc[14] = wcrc[15];
			wcrc[15] = byte2;
		}
	}
	

	// if msb byte crc = 0 then value at 27
	byte2 = 0;
	for (i = 0; i < 8; i++)
#ifdef __VCpp__  // add for Visual C++ by 7L1RLL 11/07/2015
	  byte2 = byte2 + (short) wcrc[i] * pow((double)2.0, (int)i); 
#else
	  byte2 = byte2 + wcrc[i] * pow(2, i);   
#endif
	if (byte2 == 0) byte2 =27;

	// if lsb byte crc = 0 then value at 43
	byte1 = 0;
	for (i = 8; i < 16; i++)
#ifdef __VCpp__		
	    byte1 = byte1 + (short) (wcrc[i] * (double)pow(2.0, (int)i - 8)); // add cast by 7L1RLL
#else
        byte1 = byte1 + (wcrc[i] * pow(2, i - 8)); 
#endif
	if (byte1 == 0) byte1 = 43;
	if (DEBUG) printf("byte1 = %x, byte2 = %x\n", byte1, byte2);

	// merge crc into a string
	for (i = 0; i < 8; i++)
	{
		if (i > 0) byte2 = byte2 >> 1;
		wcrc[7 - i] = byte2 & 0x01;

		if ( i > 0) byte1 = byte1 >> 1;
		wcrc[15 - i] = byte1 & 0x01;
	}
	if (length > 16) length = 16;
	for (i = 16 - length; i < 16; i++)
		crc[i - (16 - length)] = wcrc[i] + 0x30;
	crc[length] = 0x00;
} // end of genarate_crc

//*********************************************
void add_crc16(char * call_coded, char *vector)
//*********************************************
{   // input: |28 bits|, output : |51 bits|  
	char crc1[17] = "", crc2[4] = "";

#ifdef __VCpp__  // for wide character compiler
    char temp[52] = ""; 
	
	_tsetlocale(LC_ALL, _T(""));  //Change Unicode to OS-Default locale
	   
	if (DEBUG) 
	    print_str("call_coded in add CRC16 =", call_coded);
	strcpy_w(temp, call_coded, 28);    // 28 bit
	if (DEBUG) 
		print_str("temp in add CRC16=", temp);
	generate_crc(call_coded, crc1, 16);                  
	if (DEBUG) 
		print_str("crc1 =", crc1);
	strcat_w(temp, crc1, 28, 16);          // 28　+ 16 = 44
	generate_crc( temp, crc2, 3);
	if (DEBUG) 
		print_str("crc2 =", crc2);
#else  // PIC C compiler using ASCII
//	char crc1[17] = "", crc2[4] = "";
	generate_crc(call_coded, crc1, 16);
    if (DEBUG) printf("crc1      =%s\n", crc1);
	generate_crc(strcat(call_coded, crc1), crc2, 3);
    if (DEBUG) printf("crc2      =%s\n", crc2);
#endif
	// |4 bits sync| + |28 bits call| + |19 bit crc|
#ifdef __VCpp__
	strcpy_w(vector, "0000", 4);   // 4
	strcat_w(vector, temp, 4, 44);    // 4 + 44 = 48
    strcat_w(vector, crc2, 48, 3);     // 48 + 3 = 51
#else // not VC++ ex : PIC
	strcpy(vector, "0000");        //  4
	strcat(vector, call_coded);    //  4 + 44 = 48
    strcat(vector, crc2);          //  48 + 3 = 51
#endif
} // end of add_crc16

//**************************************************
void scramble(char *vector, short int *vector_to_tx)
//**************************************************
{  // encoding |51 bits|
	int i=0;

	for (i = 0; i < 51; i++)
	{
		vector_to_tx[i] = vector[i] & 0x01;
		// convert ASCII to binary
		vector_to_tx[i] = vector_to_tx[i] ^ pseudo_sequence[i];
	}
} // end of scrambling

//*************************************************************************
void Walsh_Hammered_code(short int *vector_to_tx, short int *symbol_coding)
//*************************************************************************
{   // order 8 walsh matrix codification : |119 bits|
	int data = 0, idx = 0, i = 0, j = 0;

	for (i = 0; i < 51; i += 3)
	{
		data = 0;
		for (j = 0; j < 3; j++)
			data = data + (vector_to_tx[i + j] << (2 - j));
		for (j = 0; j < 7; j++)
		{
			symbol_coding[idx] = walsh_matrix[data][j];
			idx++;
		}
	}
} //end of Walsh_Hammered_code

//***********************************************************************
void interleave(short int *symbol_coding, short int *symbol_interleaving)
//***********************************************************************
{ // interleaving : |119 bits|
	int idx = 0, i = 0, j = 0;

	idx = 0;
	for (i = 0; i < 7; i++)
	{
		for (j = i; j < 119; j += 7)
		{
			symbol_interleaving[idx]= symbol_coding[j];
			idx++;
		}
	}
} // end of interleave

//**********************************************************************
void ManchesterEncode(short int *symbol_interleaving, short int *symbol)
//**********************************************************************
{ // manchester codification : |11| + |238 bits| - |1 bit|  modified by 7L1RLL 11/07/2015
	int idx = 0;  
	int i = 0, j = 0; 

	symbol[0] = 1;
	for (i = 0; i < 119; i++)
	{
		if (symbol_interleaving[i] == 0)
		{
			symbol[idx + 1] = 1;
			symbol[idx + 2] = 0;
		}
		else
		{
			symbol[idx + 1] = 0;
			symbol[idx + 2] = 1;
		}
		idx += 2;
	}
} // end of Manchester_encode

//***************
void print_help()
//***************
{
	printf("%s\n","Usage   : OPERA_Coding_Test [? | s | [d \"callsign\"]]");
	printf("%s\n"," Normal : OPERA_Coding_Test \"callsign\"");
	printf("%s\n"," Help   : OPERA_Coding_Test ?");
	printf("%s\n"," Sample : OPERA_Coding_Test s");
    printf("%s\n"," Debug  : OPERA_Coding_Test d \"callsign\"");
    printf("%s\n"," Callsign format shall be like \"AA1AAA\". ");
	printf("%s\n"," Third character mast be a numeric character(0..9).");
} // end of help

//********************************************************************
void print_short_int(const char *caption, short int *code, int length)
//********************************************************************
{   // This is service function for debugging
	int i = 0;

	printf("%s\n", caption);
	for (i = 0; i < length; i++)
	{
		printf("%d", code[i]);
		if (((i+1) %  4) == 0) printf(" ");
		if (((i+1) % 40) == 0) printf("\n");
	}
	printf("\n");
} // end fo print_short_int


void encodepitx(short int *code, int length,float Nop)
{
    
    /*and each of the
239 symbols are transmitted by keying the transmitter as CW on and off with a symbol
rate of 0.256*n s/symbol, where n is the integer of operation mode OPn that corresponds
with the Opera frequency recommendation: */
	int SR=1e4/((0.256*Nop));
	fprintf(stderr,"SR=%d\n",SR);	
	int FifoSize=512;
	amdmasync amtest(Frequency,SR,14,FifoSize); 
	int count=0;   
	for (int i = 0; (i < length-1)&&(running==true); )
	{
			int Available=amtest.GetBufferAvailable();
			if(Available>FifoSize/2)
			{	
					int Index=amtest.GetUserMemIndex();			
					for(int j=0;j<Available;j++)
					{
							if(i==-1) 
								amtest.SetAmSample(Index+j,1);
							else
								amtest.SetAmSample(Index+j,code[i]);
							count++;
							if(count>1e4) 
							{
								count=0;
								i++;
							}
					}
							
						
			}
			else
				usleep(100);	
		
	
	}
 
}


//********************************************************************
void print_str(const char * caption, char * code)
//********************************************************************
{   // This is a service function for debugging
	size_t i = 0;

	printf("%s\n", caption);
	for (i = 0; i < strlen(code); i++)
	{ 
		printf("%c", code[i]);
		if (((i + 1) %  4) == 0) printf(" ");
		if (((i + 1) % 40) == 0) printf("\n");
	}
	printf("\n");
} // end fo print_str
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
//************** End of Program ********************************
