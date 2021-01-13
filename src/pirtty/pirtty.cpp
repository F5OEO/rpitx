#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
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
#include <librpitx/librpitx.h>

#define byte uint8_t

int MARK = 2125;
int SPACE = 1955;


#define BAUDOT_SHIFT_LTRS 0x1f
#define BAUDOT_SHIFT_FIGS 0x1b
int BAUD_RATE = 22; // 45.45 baud


char BAUDOT_FIGS[] = " \r\n1234567890-\a@!&#'()\"/:;?,.";
char BAUDOT_LTRS[] = " \r\nQWERTYUIOPASDFGHJKLZXCVBNM";
uint8_t baudot[] = {0x4,0x8,0x2,0x1d,0x19,0x10,0xa,0x1,0x15,0x1c,0xc,0x3,0xd,0x18,0x14,0x12,0x16,0xb,0x5,0x1a,0x1e,0x9,0x11,0x17,0xe,0xf,0x13,0x6,0x7};


char TXTEXT[] = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG";

















int FileFreqTiming;

ngfmdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=10000; //10ms
double frequencyshift=20000;
bool running=true;

void playtone(double Frequency,uint32_t Timing)//Timing in 0.1us
{
                uint32_t SumTiming=0;
                SumTiming+=Timing%100;
                if(SumTiming>=100)
                {
                        Timing+=100;
                        SumTiming=SumTiming-100;
                }
                int NbSamples=(Timing/100);

                while(NbSamples>0)
                {
                        usleep(10);
                        int Available=fmmod->GetBufferAvailable();
                        if(Available>FifoSize/2)
                        {
                                int Index=fmmod->GetUserMemIndex();
                                if(Available>NbSamples) Available=NbSamples;
                                for(int j=0;j<Available;j++)
                                {
                                        fmmod->SetFrequencySample(Index+j,Frequency);
                                        NbSamples--;

                                }
                        }
                }
}















void rtty_txbit (bool bit)
{
    if(running) {
        if (bit)
        {
            // MARK
            playtone(MARK, BAUD_RATE*10000);
        }
        else
        {
            // SPACE
            playtone(SPACE, BAUD_RATE*10000);
        }
    }

}


void rtty_txbyte (byte c)
{


    rtty_txbit (0); // Start bit
    // Send bits for for char MSB first
    for (byte i = 5; i > 0; --i)
    {
        if ((c & (1 << (i-1))) != 0)
            rtty_txbit(1);
        else
            rtty_txbit(0);
    }
    rtty_txbit (1); // Stop bits
    rtty_txbit (1);
}



void tx(char* string)
{
    char c = *string++;
    byte index = 0, pos = 0;
    byte shift = 0;

    while (c != '\0')
    {
        char* index = strchr(BAUDOT_FIGS, c);
        if(index != NULL) {
            pos = index - BAUDOT_FIGS;
            if(pos > 2 && shift != 1) {
                shift = 1;
                rtty_txbyte(BAUDOT_SHIFT_FIGS);
            }
            rtty_txbyte (baudot[pos]);
        } else {
            c = toupper(c);
            index = strchr(BAUDOT_LTRS, c);
            pos = index - BAUDOT_LTRS;
            if(pos > 2 && shift != 2) {
                shift = 2;
                rtty_txbyte(31);
            }
            rtty_txbyte (baudot[pos]);
        }
        c = *string++;
    }
}





























static void SendTones()
{
double basefreq = 0;
double freq2 = 1100 + frequencyshift;
//while(running)
//{
//        tone(1100);
//}
tx(TXTEXT);
}

static void
terminate(int num)
{
    running=false;
        fprintf(stderr,"Caught signal - Terminating %x\n",num);

}

int main(int argc, char **argv)
{
        float frequency=144.5e6;
        if (argc > 3)
        {


                 frequency=atof(argv[1]);
                 SPACE = atoi(argv[2]);
                 MARK = SPACE + 170;
                 strcpy(TXTEXT, argv[3]);

        }
        else
        {
                printf("usage : pirtty [frequency(Hz)] [Space Frequency(hz)] [text]\n");
                exit(0);
        }

        for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

        fmmod=new ngfmdmasync(frequency,100000,14,FifoSize);
        SendTones();
        delete fmmod;
        return 0;
}




