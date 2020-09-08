#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "ssb_gen.h"
#include <sndfile.h>

#define		BUFFER_LEN	1024*8

// Test program using SNDFILE 
// see http://www.mega-nerd.com/libsndfile/api.html for API

int main(int argc, char **argv) 
{
	float data [2*BUFFER_LEN];
	float data_filtered[2*BUFFER_LEN]; // we generate complex I/Q samples
	SNDFILE      *infile, *outfile;
	SF_INFO		sfinfo;
	SF_INFO		sf_out;
	int			readcount, nb_samples;
	char	*infilename;
	char	*outfilename;
	int k;
	float x,I,Q;
	
	if( argc < 2 ) {
		printf("Usage : %s in.wav [out.wav]\n", argv[0]);
		return(1);
	}	
	infilename = argv[1];
	if( argc == 3 ) {
		outfilename = argv[2];
	} else {
		outfilename = (char *)malloc( 128 );
		sprintf( outfilename, "%s", "out.wav");
	}
	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{
		/* Open failed so print an error message. */
		printf ("Not able to open input file %s.\n", infilename);
		/* Print the error message from libsndfile. */
		puts (sf_strerror (NULL));
		return  1;
	}
	
	if( sfinfo.samplerate != 48000 ) {
		printf("Input rate must be 48K.\n");
		return 1;
	}
 
	memcpy( (void *)&sf_out, (void *)&sfinfo, sizeof( SF_INFO ));
	sf_out.channels = 2;
	//sf_out.samplerate = sfinfo.samplerate/3;
	/* Open the output file. */
	if (! (outfile = sf_open (outfilename, SFM_WRITE, &sf_out)))
	{	
		printf ("Not able to open output file %s.\n", outfilename);
		puts (sf_strerror (NULL));
        return  1;
	}
		
	/** **/
	printf ("Reading file : %s\n", infilename );
	printf ("Sample Rate : %d\n", sfinfo.samplerate);
	printf ("Channels    : %d\n", sfinfo.channels);
	printf ("----------------------------------------\n");
	printf ("Writing file : %s\n", outfilename );
	printf ("Channels    : %d\n",  sf_out.channels);
	
	// la porteuse SSB est décalée de +1K
	ssb_init( 000 );
	
	/* While there are.frames in the input file, read them, process
	** them and write them to the output file.
	*/
	while ((readcount = sf_readf_float(infile, data, BUFFER_LEN)))
	{   
		nb_samples = readcount / sfinfo.channels;		
		for( k=0 ; k < nb_samples ; k++ ) {
			x = data[k*sfinfo.channels];
			if( sfinfo.channels == 2 ) {
				// stereo file, avg left + right
				x += data[k*sfinfo.channels+1];
				x /= 2;
			}
			// voir ssb_gen.h, mettre MODULE_SSB_LSB pour LSB module
			ssb( x, MODULE_SSB_USB , &I, &Q );
			data_filtered[2*k  ] = I; //I and Q seems to be between 0 and 0.5
			data_filtered[2*k+1] = Q;	
			/* // FOR COMPRESSOR *2
			data_filtered[2*k  ] = I * 2; //I and Q seems to be between 0 and 0.5
			data_filtered[2*k+1] = Q * 2;	
			*/
		}
		sf_write_float(outfile, data_filtered, 2*nb_samples );
	}

    /* Close input and output files. */
    sf_close (infile);
    sf_close (outfile);

    return 0;
}
