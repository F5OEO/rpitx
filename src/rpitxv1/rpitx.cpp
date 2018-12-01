/*<rpitx is a software which use the GPIO of Raspberry Pi to transmit HF>
    This version is to allow back compatibility with rpitx v1
    Copyright (C) 2015-2018  Evariste COURJAUD F5OEO (evaristec@gmail.com)
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
#include <unistd.h>
#include "../librpitx/src/librpitx.h"
#include "stdio.h"
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <cstring>
#include <signal.h>
#include <stdlib.h>


#define PROGRAM_VERSION "2.0"

bool running=true;

void print_usage(void)
{
fprintf(stderr,"Warning : rpitx V2 is only to try to be compatible with version 1\n");

fprintf(stderr,\
"\nrpitx -%s\n\
Usage:\nrpitx [-i File Input][-m ModeInput] [-f frequency output] [-s Samplerate] [-l] [-p ppm] [-h] \n\
-m            {IQ(FileInput is a Stereo Wav contains I on left Channel, Q on right channel)}\n\
              {IQFLOAT(FileInput is a Raw float interlaced I,Q)}\n\
              {RF(FileInput is a (double)Frequency,Time in nanoseconds}\n\
       	      {RFA(FileInput is a (double)Frequency,(int)Time in nanoseconds,(float)Amplitude}\n\
	      {VFO (constant frequency)}\n\
-i            path to File Input \n\
-f float      frequency to output on GPIO_4 pin 7 in khz : (130 kHz to 750 MHz),\n\
-l            loop mode for file input\n\
-p float      frequency correction in parts per million (ppm), positive or negative, for calibration, default 0.\n\
-h            help (this help).\n\
\n",\
PROGRAM_VERSION);

} /* end function print_usage */

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating %x\n",num);
   
}

static void fatal(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	terminate(0);
}

int main(int argc, char* argv[])
{
    enum {MODE_RPITX_IQ=0,MODE_RPITX_RF,MODE_RPITX_RFA,MODE_RPITX_IQ_FLOAT,MODE_RPITX_VFO};
	int a;
	int anyargs = 0;
	char Mode = MODE_IQ;  // By default
	int SampleRate=48000;
	float SetFrequency=1e6;//1MHZ
	float ppmpll=0.0;
	int SetDma=0;
    char *FileName=NULL;
    FILE *FileInHandle=NULL;
    bool loop_mode_flag=false;
    bool useStdin;
    int Harmonic=1;
	while(1)
	{
		a = getopt(argc, argv, "i:f:m:s:p:hld:w:c:ra:");
	
		if(a == -1) 
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(a)
		{
		case 'i': // File name
			FileName = optarg;
			break;
		case 'f': // Frequency
			SetFrequency = atof(optarg)*1e3;
			break;
		case 'm': // Mode (IQ,IQFLOAT,RF,RFA)
			if(strcmp("IQ",optarg)==0) Mode=MODE_RPITX_IQ;
			if(strcmp("RF",optarg)==0) Mode=MODE_RPITX_RF;	
			if(strcmp("RFA",optarg)==0) Mode=MODE_RPITX_RFA;
			if(strcmp("IQFLOAT",optarg)==0) Mode=MODE_RPITX_IQ_FLOAT;
			if(strcmp("VFO",optarg)==0) Mode=MODE_RPITX_VFO;
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
			loop_mode_flag = true;
			break;
		case 'd': // Dma Sample Burst
			fprintf(stderr,"Warning : 'd' parameter not used in this version\n");
			break;
		case 'c': fprintf(stderr,"Warning : 'c' parameter not used in this version\n");
			break;
		case 'w': // No use pwmfrequency 
				fprintf(stderr,"Warning : 'w' parameter not used in this version\n");
			break;
		case 'r': // Randomize PWM frequency 
				fprintf(stderr,"Warning : 'r' parameter not used in this version\n");
			
			break;
		case 'a': // DMA Channel 1-14
			 	fprintf(stderr,"Warning : 'a' parameter not used in this version\n");
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

	//Open File Input for modes which need it
	if((Mode==MODE_RPITX_IQ)||(Mode==MODE_RPITX_IQ_FLOAT)||(Mode==MODE_RPITX_RF)||(Mode==MODE_RPITX_RFA))
	{
		if(FileName && strcmp(FileName,"-")==0)
		{
			FileInHandle = stdin;
			useStdin = true;
		}
		else FileInHandle = fopen(FileName	,"rb");
		if (FileInHandle ==NULL)
		{
			fatal("Failed to read Filein %s\n",FileName);
		}
	}

    for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

    fprintf(stderr,"Warning : rpitx V2 is only to try to be compatible with version 1\n");
    // For IQ
    #define IQBURST 4000
    iqdmasync *iqsender=NULL;
    std::complex<float> CIQBuffer[IQBURST];	
    int Decimation=1;
    // For RF (FM)
    //For RFA (AM)
    amdmasync *amsender=NULL;
    ngfmdmasync *fmsender=NULL;
    float AmOrFmBuffer[IQBURST];	
    int FifoSize=IQBURST*4;
    //Init
    switch(Mode)
    {
            case MODE_RPITX_IQ:
            case MODE_RPITX_IQ_FLOAT:
            {
                
                
                
                iqsender=new iqdmasync(SetFrequency,SampleRate,14,FifoSize,MODE_IQ); 
                iqsender->Setppm(ppmpll);  
            }
            break;
            case MODE_RPITX_RFA://Amplitude
            {
                amsender=new amdmasync(SetFrequency,SampleRate,14,FifoSize);
            }
            break;
            case MODE_RPITX_RF://Frequency
            {
                fmsender=new ngfmdmasync(SetFrequency,SampleRate,14,FifoSize);
            }

    }        

	//resetFile();
	//return pitx_run(Mode, SampleRate, SetFrequency, ppmpll, NoUsePwmFrequency, readFile, resetFile, NULL,SetDma);
    while(running)
    {
        switch(Mode)
        {
            case MODE_RPITX_IQ:
            case MODE_RPITX_IQ_FLOAT:
            {
                
                int CplxSampleNumber=0;    
                
                switch(Mode)
                {
                    case MODE_RPITX_IQ://I16
                    {
                        static short IQBuffer[IQBURST*2];
                       
                        int nbread=fread(IQBuffer,sizeof(short),IQBURST*2,FileInHandle);
                        //if(nbread==0) continue;
                        if(nbread>0)
                        {
                            for(int i=0;i<nbread/2;i++)
                            {
                                if(i%Decimation==0)
                                {		
                                    CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2]/32768.0,IQBuffer[i*2+1]/32768.0); 
                                }
                            }
                        }
                        else 
                        {
                            printf("End of file\n");
                            if(loop_mode_flag&&!useStdin)
                            fseek ( FileInHandle , 0 , SEEK_SET );
                            else
                                running=false;
                        }
                        
                    }
                    break;
                    case MODE_RPITX_IQ_FLOAT:
                    {
                        static float IQBuffer[IQBURST*2];
                        int nbread=fread(IQBuffer,sizeof(float),IQBURST*2,FileInHandle);
                        //if(nbread==0) continue;
                        if(nbread>0)
                        {
                            for(int i=0;i<nbread/2;i++)
                            {
                                if(i%Decimation==0)
                                {	
                                    CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2],IQBuffer[i*2+1]);
                                            
                                }		 
                                //printf("%f %f\n",(IQBuffer[i*2]-127.5)/128.0,(IQBuffer[i*2+1]-127.5)/128.0);
                            }
                        }
                        else 
                        {
                            printf("End of file\n");
                            if(loop_mode_flag&&useStdin)
                            fseek ( FileInHandle , 0 , SEEK_SET );
                            else
                                running=false;
                        }
                    }
                    break;	
                    
                }    
                iqsender->SetIQSamples(CIQBuffer,CplxSampleNumber,Harmonic);    
            }
                
            break;
            
            case MODE_RPITX_RFA://Amplitude
            case MODE_RPITX_RF://Frequence
            {
                
                        typedef struct {
		                 double Frequency;
		                uint32_t WaitForThisSample;
	                    } samplerf_t;

                        int SampleNumber=0;    
                        static samplerf_t RfBuffer[IQBURST];
                        int nbread=fread(RfBuffer,sizeof(samplerf_t),IQBURST,FileInHandle);
                        //if(nbread==0) continue;
                        if(nbread>0)
                        {
                            for(int i=0;i<nbread;i++)
                            {
                                    AmOrFmBuffer[SampleNumber++]=(float)(RfBuffer[i].Frequency);
                                            
                            }
                        }
                        else 
                        {
                            printf("End of file\n");
                            if(loop_mode_flag&&useStdin)
                            fseek ( FileInHandle , 0 , SEEK_SET );
                            else
                                running=false;
                        }
                switch(Mode)
                {
                    case MODE_RPITX_RFA:    
                    {
                        amsender->SetAmSamples(AmOrFmBuffer,SampleNumber);
                    }
                    break;
                    case MODE_RPITX_RF:    
                    {
                        fmsender->SetFrequencySamples(AmOrFmBuffer,SampleNumber);
                    }
                    break;
                }    

            }
            break;
            case MODE_RPITX_VFO:
            {

            }
            break;

        }
    }
    // This is the end
    switch(Mode)
    {
        case MODE_RPITX_IQ:
        case MODE_RPITX_IQ_FLOAT:delete(iqsender);
        break;
        case MODE_RPITX_RFA:delete(amsender);break;
        case MODE_RPITX_RF:delete(fmsender);break;
    }

}
