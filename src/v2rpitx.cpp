#include <unistd.h>
#include "librpitx.h"
#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>

bool running=true;
void SimpleTest(uint64_t Freq)
{
	

	clkgpio clk;
	clk.print_clock_tree();
	//clk.SetPllNumber(clk_plla,0);
	clk.SetAdvancedPllMode(true);
	clk.SetCenterFrequency(Freq,100000);
	clk.SetFrequency(000);
	clk.enableclk(4);
	sleep(60);
	/*for(int i=0;i<100000;i+=1)
	{
		clk.SetFrequency(i);
		usleep(1000);
	}*/
	clk.disableclk(4);
	
}

void SimpleTestDMA(uint64_t Freq)
{
	

	int SR=200000;
	int FifoSize=4096;
	//ngfmdmasync ngfmtest(1244200000,SR,14,FifoSize);
	ngfmdmasync ngfmtest(Freq,SR,14,FifoSize);
	for(int i=0;running;)
	{
		//usleep(10);
		usleep(FifoSize*1000000.0*3.0/(4.0*SR));
		int Available=ngfmtest.GetBufferAvailable();
		if(Available>FifoSize/2)
		{	
			int Index=ngfmtest.GetUserMemIndex();
			//printf("GetIndex=%d\n",Index);
			for(int j=0;j<Available;j++)
			{
				//ngfmtest.SetFrequencySample(Index,((i%10000)>5000)?1000:0);
				ngfmtest.SetFrequencySample(Index+j,(i%SR));
				i++;
			
			}
		}
		
		
	}
	fprintf(stderr,"End\n");
	
	ngfmtest.stop();
	
}
using std::complex;
void SimpleTestLiquid()
{
	

	int SR=200000;
	int FifoSize=4096;
	ngfmdmasync ngfmtest(144200000,SR,14,FifoSize);
	dsp mydsp(SR);
	 nco_crcf q = nco_crcf_create(LIQUID_NCO);
	 nco_crcf_set_phase(q, 0.0f);
	nco_crcf_set_frequency(q, -0.2f);
	
	//ngfmtest.print_clock_tree();
	for(int i=0;(i<SR)&&running;)
	{
		//usleep(10);
		usleep(FifoSize*1000000.0*3.0/(4.0*SR));
		int Available=ngfmtest.GetBufferAvailable();
		if(Available>FifoSize/2)
		{	
			int Index=ngfmtest.GetUserMemIndex();
			//printf("GetIndex=%d\n",Index);
			for(int j=0;j<Available;j++)
			{
				//ngfmtest.SetFrequencySample(Index,((i%10000)>5000)?1000:0);
				//ngfmtest.SetFrequencySample(Index+j,(i%SR));
				nco_crcf_adjust_frequency(q,1e-5);
				liquid_float_complex x;
				nco_crcf_step(q);
				nco_crcf_cexpf(q, &x);
				mydsp.pushsample(x);
				if(mydsp.frequency>SR) mydsp.frequency=SR;
				if(mydsp.frequency<-SR) mydsp.frequency=-SR;
				ngfmtest.SetFrequencySample(Index+j,mydsp.frequency);
				//fprintf(stderr,"freq=%f Amp=%f\n",mydsp.frequency,mydsp.amplitude);
				//fprintf(stderr,"freq=%f\n",nco_crcf_get_frequency(q)*SR);
				i++;
			
			}
		}
		
		
	}
	fprintf(stderr,"End\n");
	
	ngfmtest.stop();
}

void SimpleTestFileIQ(uint64_t Freq)
{
	FILE *iqfile=NULL;
	iqfile=fopen("../ssbtest.iq","rb");
	if (iqfile==NULL) printf("input file issue\n");


	bool stereo=true;
	int SR=48000;
	int FifoSize=512;
	//iqdmasync iqtest(1245000000,SR,14,FifoSize);
	//iqdmasync iqtest(50100000,SR,14,FifoSize);
	iqdmasync iqtest(Freq,SR,14,FifoSize);
	//iqdmasync iqtest(14100000,SR,14,FifoSize);
	short IQBuffer[128*2];
	
	while(running)
	{
		//usleep(FifoSize*1000000.0*1.0/(8.0*SR));
		usleep(100);
		int Available=iqtest.GetBufferAvailable();
		if(Available>256)
		{	
			int Index=iqtest.GetUserMemIndex();
			int nbread=fread(IQBuffer,sizeof(short),128*2,iqfile);
			if(nbread>0)
			{
				//printf("NbRead=%d\n",nbread);
				for(int i=0;i<nbread/2;i++)
				{
					
					liquid_float_complex x=complex<float>(IQBuffer[i*2]/32768.0,IQBuffer[i*2+1]/32768.0); 
					iqtest.SetIQSample(Index+i,x);
					
				}
			}
			else 
			{
				printf("End of file\n");
				fseek ( iqfile , 0 , SEEK_SET );
				//break;
			}
		}
	}
	iqtest.stop();
}


void SimpleTestbpsk(uint64_t Freq)
{
	
	
	clkgpio clk;
	clk.print_clock_tree();
	int SR=1000;
	int FifoSize=1024;
	int NumberofPhase=2;
	phasedmasync biphase(Freq,SR,NumberofPhase,14,FifoSize);
	int lastphase=0;
	while(running)
	{
		//usleep(FifoSize*1000000.0*1.0/(8.0*SR));
		usleep(10);
		int Available=biphase.GetBufferAvailable();
		if(Available>256)
		{	
			int Index=biphase.GetUserMemIndex();
			
				/*
				for(int i=0;i<Available;i++)
				{
					int phase=(rand()%NumberofPhase);
					biphase.SetPhase(Index+i,phase);
				}*/
				/* 					
				for(int i=0;i<Available/2;i++)
				{
					int phase=2*(rand()%NumberofPhase/2);
					biphase.SetPhase(Index+i*2,(phase+lastphase)/2);
					biphase.SetPhase(Index+i*2+1,phase);
					lastphase=phase;
				}*/
				for(int i=0;i<Available;i++)
				{
					lastphase=(lastphase+1)%NumberofPhase;
					biphase.SetPhase(Index+i,lastphase);
				}
			
		}
	}
	biphase.stop();
}


void SimpleTestSerial()
{
	
	bool stereo=true;
	int SR=10000;
	int FifoSize=1024;
	bool dualoutput=true;
	serialdmasync testserial(SR,14,FifoSize,dualoutput);
	
	while(running)
	{
		
		usleep(10);
		int Available=testserial.GetBufferAvailable();
		if(Available>256)
		{	
			int Index=testserial.GetUserMemIndex();
			
				
				for(int i=0;i<Available;i++)
				{
					
					
					testserial.SetSample(Index+i,i);
					
				}
			
		}
	}
	testserial.stop();
}

static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating\n");
   
}


int main(int argc, char* argv[])
{
	
	uint64_t Freq=144200000;
	if(argc>1)
		 Freq=atol(argv[1]);

	 for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	//SimpleTest(Freq);
	//SimpleTestbpsk(Freq);
	//SimpleTestFileIQ(Freq);
	SimpleTestDMA(Freq);
	
}	

