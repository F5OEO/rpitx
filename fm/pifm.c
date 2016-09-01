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

#include <sndfile.h>

#define		BUFFER_LEN	1024*8
int FileFreqTiming;
// Test program using SNDFILE
// see http://www.mega-nerd.com/libsndfile/api.html for API

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
	if (write(FileFreqTiming, &RfSample, sizeof(samplerf_t)) != sizeof(samplerf_t)) {
		fprintf(stderr, "Unable to write sample\n");
	}
}

int main(int argc, char **argv)
{
	float data [2*BUFFER_LEN];
	float data_filtered[2*BUFFER_LEN]; // we generate complex I/Q samples
	SNDFILE	*infile, *outfile;
	SF_INFO	sfinfo;

	int readcount, nb_samples;
	char *infilename;
	char *outfilename;
	int k;
	float x;

	if( argc < 2 ) {
		printf("Usage : %s in.wav [out.wav]\n", argv[0]);
		return(1);
	}
	infilename = argv[1];
	if( argc == 3 ) {
		outfilename = argv[2];
	} else {
		outfilename = (char *)malloc( 128 );
		sprintf( outfilename, "%s", "out.ft");
	}
    if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
    {   /* Open failed so print an error message. */
        printf ("Not able to open input file %s.\n", infilename);
        /* Print the error message from libsndfile. */
        puts (sf_strerror (NULL));
        return  1;
    }

	if( sfinfo.samplerate != 48000 ) {
		printf("Input rate must be 48K.\n");
		return 1;
	}

	FileFreqTiming = open(outfilename, O_WRONLY|O_CREAT, 0644);

	/** **/
	printf ("Reading file : %s\n", infilename ) ;
	printf ("Sample Rate : %d\n", sfinfo.samplerate) ;
	printf ("Channels    : %d\n", sfinfo.channels) ;
	printf ("----------------------------------------\n") ;
	printf ("Writing file : %s\n", outfilename ) ;

	/* While there are.frames in the input file, read them, process
    ** them and write them to the output file.
    */
	int Excursion=6000;
    while ((readcount = sf_readf_float(infile, data, BUFFER_LEN)))
    {
		nb_samples = readcount / sfinfo.channels ;
		for( k=0 ; k < nb_samples ; k++ ) {
			x = data[k*sfinfo.channels] ;
			if( sfinfo.channels == 2 ) {
				// stereo file, avg left + right
				x += data[k*sfinfo.channels+1] ;
				x /= 2 ;
			}
			//printf("%f \n",x);
			WriteTone(x*Excursion*2.0,1e9/48000.0);
		}
	}

    /* Close input and output files. */
    sf_close (infile) ;
	close(FileFreqTiming);

	return 0;
}
