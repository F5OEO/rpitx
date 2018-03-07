#include <unistd.h>
#include "librpitx.h"
#include <unistd.h>
#include "stdio.h"

void SimpleTest()
{
	generalgpio generalio;
	generalio.enableclk();


	clkgpio clk;
	clk.SetPllNumber(clk_plld,1);
	clk.SetAdvancedPllMode(true);
	clk.SetCenterFrequency(144100000);
	for(int i=0;i<1000000;i+=1)
	{
		clk.SetFrequency(i);
		//usleep(1);
	}
	
	
}

void SimpleTestDMA()
{
	generalgpio generalio;
	generalio.enableclk();

	int SR=200000;
	int FifoSize=4096;
	ngfmdmasync ngfmtest(1244200000,SR,14,FifoSize);
	
	for(int i=0;i<SR;)
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
				ngfmtest.SetFrequencySample(Index,((i%10000)>5000)?1000:0);
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
	generalgpio generalio;
	generalio.enableclk();

	int SR=200000;
	int FifoSize=4096;
	ngfmdmasync ngfmtest(144200000,SR,14,FifoSize);
	dsp mydsp(SR);
	 nco_crcf q = nco_crcf_create(LIQUID_NCO);
	 nco_crcf_set_phase(q, 0.0f);
	nco_crcf_set_frequency(q, -0.2f);
	
	//ngfmtest.print_clock_tree();
	for(int i=0;i<SR;)
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

void SimpleTestFileIQ()
{
	FILE *iqfile=NULL;
	iqfile=fopen("ssb.iq","rb");
	if (iqfile==NULL) printf("input file issue\n");

	generalgpio generalio;
	generalio.enableclk();

	int SR=48000;
	int FifoSize=512;
	iqdmasync iqtest(1242300000,SR,14,FifoSize);
	
	short IQBuffer[128*2];
	printf("Sizeof = %d\n",sizeof(liquid_float_complex));
	while(1)
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

int main(int argc, char* argv[])
{
	SimpleTestFileIQ();
	
	
}	

