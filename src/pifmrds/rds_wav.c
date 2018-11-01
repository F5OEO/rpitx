/*
    PiFmRds - FM/RDS transmitter for the Raspberry Pi
    Copyright (C) 2014 Christophe Jacquet, F8FTK
    
    See https://github.com/ChristopheJacquet/PiFmRds
    
    rds_wav.c is a test program that writes a RDS baseband signal to a WAV
    file. It requires libsndfile.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>
#include <string.h>

#include "rds.h"
#include "fm_mpx.h"


#define LENGTH 114000


/* Simple test program */
int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Error: missing argument.\n");
        fprintf(stderr, "Syntax: rds_wav <in_audio.wav> <out_mpx.wav> <text>\n");
        return EXIT_FAILURE;
    }
    
    set_rds_pi(0x1234);
    set_rds_ps(argv[3]);
    set_rds_rt(argv[3]);
    
    char *in_file = argv[1];
    if(strcmp("NONE", argv[1]) == 0) in_file = NULL;
    
    if(fm_mpx_open(in_file, LENGTH) != 0) {
        printf("Could not setup FM mulitplex generator.\n");
        return EXIT_FAILURE;
    }
    

    
    // Set the format of the output file
    SNDFILE *outf;
    SF_INFO sfinfo;

    sfinfo.frames = LENGTH;
    sfinfo.samplerate = 228000;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfo.sections = 1;
    sfinfo.seekable = 0;
    
    // Open the output file
    char *out_file = argv[2];
    if (! (outf = sf_open(out_file, SFM_WRITE, &sfinfo))) {
        fprintf(stderr, "Error: could not open output file %s.\n", out_file);
        return EXIT_FAILURE;
    }

    float mpx_buffer[LENGTH];

    for(int j=0; j<40; j++) {
        if( fm_mpx_get_samples(mpx_buffer) < 0 ) break;
        
        // scale samples
        for(int i=0; i<LENGTH; i++) {
            mpx_buffer[i] /= 10.;
        }

        if(sf_write_float(outf, mpx_buffer, LENGTH) != LENGTH) {
            fprintf(stderr, "Error: writing to file %s.\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    
    if(sf_close(outf) ) {
        fprintf(stderr, "Error: closing file %s.\n", argv[1]);
    }
    
    fm_mpx_close();

    return EXIT_SUCCESS;
}
