//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//	  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//	  of conditions and the following disclaimer in the documentation and/or other materials
//	  provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Sylvain AZARIAN F4GKR ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sylvain AZARIAN OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Sylvain AZARIAN F4GKR.
//==========================================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "ssb_gen.h"

struct FIR {
    float *coeffs;
    int filterLength;
    float *delay_line;
};

typedef struct _cpx {
	float re;
	float im;
} TYPECPX;

struct cFIR {
    float *coeffs;
    int filterLength;
    TYPECPX *delay_line;
	float rms;
};

#define ALPHA_DC_REMOVE (0.999)

struct FIR* audio_fir;
struct FIR* hilbert;
struct FIR* delay;
struct cFIR* interpolateIQ; 
 
// Create a FIR struct, used to store a copy of coeffs and delay line
// in : coeffs_len = coeff tab length
//      double *coeff_tab = pointer to the coefficients
// out: a struct FIR
struct FIR* init_fir( int coeffs_len, float *coeff_tab ) 
{
	struct FIR *result;
	int i;
	// alloc and init buffers
	result = (struct FIR *)malloc( sizeof( struct FIR ));
	result->filterLength = coeffs_len;
	result->coeffs = (float*)malloc( coeffs_len * sizeof( float));
	result->delay_line = (float*)malloc( coeffs_len * sizeof( float));
	// copy coeffs to struct
	for( i=0 ; i < coeffs_len ; i++ ) {
		result->delay_line[i] = 0.0;
		result->coeffs[i] = coeff_tab[i];
	}
	return( result );
}

// init a complex in -> complex out fir with real coeffs
struct cFIR* init_cfir( int coeffs_len, float *coeff_tab ) 
{
	struct cFIR *result;
	int i;
	// alloc and init buffers
	result = (struct cFIR *)malloc( sizeof( struct cFIR ));
	result->filterLength = coeffs_len ;
	result->coeffs = (float*)malloc( coeffs_len * sizeof( float));
	result->delay_line = (TYPECPX*)malloc( coeffs_len * sizeof( TYPECPX));
	// copy coeffs to struct
	for( i=0 ; i < coeffs_len ; i++ ) {
		result->delay_line[i].re = 0.0;
		result->delay_line[i].im = 0.0;
		result->coeffs[i] = coeff_tab[i];
	}
	return( result );
}

float fir_filt( struct FIR* f, float in ) 
{
	int i;
	float acc; 
	float *pt_coeffs;
	float *pt_sample;
	int L;
	
	// shift input left for one sample
	L = f->filterLength;
	for( i=0 ; i < L - 1 ; i++ ) {
		 f->delay_line[i] = f->delay_line[i+1];
	}
	// add new sample to the end of delay line
	f->delay_line[ L - 1 ] = in;
	acc = 0 ;
	pt_sample = f->delay_line;
	pt_coeffs = f->coeffs;
	// do the compute loop
	for( i=0 ; i < L ; i++ ) {
		acc += (*pt_sample) * (*pt_coeffs );
		
		pt_sample++;
		pt_coeffs++;
	}
	return( acc );
}
// same but we filter a complex number at input, out is a complex
TYPECPX cfir_filt( struct cFIR* f, TYPECPX in ) 
{
	int i;
	TYPECPX acc; 
	float *pt_coeffs, rms;
	TYPECPX *pt_sample;
	int L;
	
	// shift input left for one sample
	L = f->filterLength;
	for( i=0 ; i < L - 1 ; i++ ){
		 f->delay_line[i] = f->delay_line[i+1];
	}
	// add new sample to the end of delay line
	f->delay_line[ L - 1 ] = in;
	acc.re = acc.im = 0;
	pt_sample = f->delay_line;
	pt_coeffs = f->coeffs;
	// do the compute loop	
	rms = 0;
	for( i=0 ; i < L ; i++ ) {
		acc.re += (pt_sample->re) * (*pt_coeffs );
		acc.im += (pt_sample->im) * (*pt_coeffs );
		rms += (pt_sample->re)*(pt_sample->re) + (pt_sample->im)*(pt_sample->im);
		pt_sample++;
		pt_coeffs++;
	}
	f->rms = sqrt( rms / L );
	return( acc );
}
 

TYPECPX m_Osc1;
double m_OscCos, m_OscSin;
int nco_enabled;
#define B_SIZE (512)
#define COMP_ATTAK ( exp(-1/48.0)) /* 0.1 ms */
#define COMP_RELEASE (exp(-1/(30*480.0))) /* 300 ms */
#define threshold (.25)

void ssb(float in, int USB, float* out_I, float* out_Q) 
{
	static float y_n1 = 0;
	static float x_n1 = 0;
	float y, Imix, Qmix, OscGn; 
	static float I=0;
	static float Q=0;	
	static int OL = 3;
	// RingBuffer management
	static int b_start = 0;
	static int b_end   = 0;
	static float elems[B_SIZE]; // power of 2, approx 1ms at 48KHz
	static int first = B_SIZE;	// this says how many samples we wait before audio processing
	static float env = 0;
	float rms, theta, gain;
	//---------------------------
	int i;
	TYPECPX dtmp, Osc, IQ;
	OL++;
	OL = OL % 4;
	//---------- lowpass filter audio input	
	// suppress DC, high pass filter
	// y[n] = x[n] - x[n-1] + alpha * y[n-1]
	y = in - x_n1 + ALPHA_DC_REMOVE*y_n1;
	x_n1 = in;
	y_n1 = y;
	
	// low pass filter y to keep only audio band	
	y = fir_filt( audio_fir, y );

//#define AUDIO_COMPRESSOR //???
#ifdef AUDIO_COMPRESSOR //???
	//----------- audio compressor	
	//--- code inspired from http://www.musicdsp.org/showone.php?id=169
	// store in our ring buffer
	if( b_end != (b_start ^ B_SIZE )) { // ring buffer not full
		elems[b_end & (B_SIZE-1)] = y; // append at the end
		
		if( b_end == (b_start ^ B_SIZE )) {
			b_start = (b_start+1)&(2*B_SIZE-1);
		}
		b_end = (b_end+1)&(2*B_SIZE-1);
	}
	// wait to have at least 2ms before starting
	if( first > 0 ) {
		first--;
		*out_I = 0;
		*out_Q = 0;
		return;
	}
	// compute RMS power in buffer
	rms = 0;
	for( i=0 ; i < B_SIZE ; i++ ) {
		rms += elems[i] * elems[i];
	}
	rms = sqrt( rms / B_SIZE );
	theta = rms > env ? COMP_ATTAK : COMP_RELEASE;
	env = (1-theta) * rms + theta * env;
	gain = 1;
	if( env > threshold ) {
		gain = 1 - (env - threshold); 
	}
	// retrieve the oldest sample
	y = elems[b_start&(B_SIZE-1)];
	b_start = (b_start+1)&(2*B_SIZE-1);
	// apply compressor gain
	//printf("%f,%f\n", env, gain );

	y *= gain ; // To enable compressor
#endif

//----------- SSB modulator stage	
	// decimation by 4
	if( OL == 0 ) {		
		// we come here 1/4 of time				
		// pass audio sample to delay line, pass band filter
		I = fir_filt( delay, y );
		// pass audio sample to hilbert transform to shift 90 degrees
		Q = fir_filt( hilbert, y );			
	} 
	
	IQ.re = I;
	IQ.im = USB * Q;
	// interpolation by 4
	dtmp = cfir_filt( interpolateIQ, IQ ); 
	// shift in freq if enabled (see ssb_init )
	if( nco_enabled ) {
		// our SSB signal is now centered at 0
		// update our NCO for shift
		Osc.re = m_Osc1.re * m_OscCos - m_Osc1.im * m_OscSin;
		Osc.im = m_Osc1.im * m_OscCos + m_Osc1.re * m_OscSin;
		OscGn = 1.95 - (m_Osc1.re*m_Osc1.re + m_Osc1.im*m_Osc1.im);
		m_Osc1.re = OscGn * Osc.re;
		m_Osc1.im = OscGn * Osc.im;			
		//Cpx multiply by shift OL
		Imix = ((dtmp.re * Osc.re) - (dtmp.im * Osc.im));
		Qmix = ((dtmp.re * Osc.im) + (dtmp.im * Osc.re));		
		*out_I = Imix; 
		*out_Q = Qmix; 
	} else {
		*out_I = dtmp.re; 
		*out_Q = dtmp.im; 
	}
}

#define SAMPLE_RATE (48000.0f)

void ssb_init( float shift_carrier)
{
	double m_NcoInc;
	float a[83];
	float b[89];
	float c[89]; 
	/*
	 * Kaiser Window FIR Filter
	 * Passband: 0.0 - 3000.0 Hz
	 * Order: 83
	 * Transition band: 3000.0 Hz
	 * Stopband attenuation: 80.0 dB
	 */
	a[0] =	-1.7250879E-5f;
	a[1] =	-4.0276995E-5f;
	a[2] =	-5.6314686E-5f;
	a[3] =	-4.0164417E-5f;
	a[4] =	3.0053454E-5f;
	a[5] =	1.5370155E-4f;
	a[6] =	2.9180944E-4f;
	a[7] =	3.6717512E-4f;
	a[8] =	2.8903902E-4f;
	a[9] =	3.1934875E-11f;
	a[10] =	-4.716546E-4f;
	a[11] =	-9.818495E-4f;
	a[12] =	-0.001290066f;
	a[13] =	-0.0011395542f;
	a[14] =	-3.8172887E-4f;
	a[15] =	9.0173044E-4f;
	a[16] =	0.0023420234f;
	a[17] =	0.003344623f;
	a[18] =	0.003282209f;
	a[19] =	0.0017731993f;
	a[20] =	-0.0010558856f;
	a[21] =	-0.004450674f;
	a[22] =	-0.0071515352f;
	a[23] =	-0.007778209f;
	a[24] =	-0.0053855875f;
	a[25] =	-2.6561373E-10f;
	a[26] =	0.0070972904f;
	a[27] =	0.013526209f;
	a[28] =	0.016455514f;
	a[29] =	0.013607533f;
	a[30] =	0.0043148645f;
	a[31] =	-0.009761283f;
	a[32] =	-0.02458954f;
	a[33] =	-0.03455451f;
	a[34] =	-0.033946108f;
	a[35] =	-0.018758629f;
	a[36] =	0.011756961f;
	a[37] =	0.054329403f;
	a[38] =	0.10202855f;
	a[39] =	0.14574805f;
	a[40] =	0.17644218f;
	a[41] =	0.18748334f;
	a[42] =	0.17644218f;
	a[43] =	0.14574805f;
	a[44] =	0.10202855f;
	a[45] =	0.054329403f;
	a[46] =	0.011756961f;
	a[47] =	-0.018758629f;
	a[48] =	-0.033946108f;
	a[49] =	-0.03455451f;
	a[50] =	-0.02458954f;
	a[51] =	-0.009761283f;
	a[52] =	0.0043148645f;
	a[53] =	0.013607533f;
	a[54] =	0.016455514f;
	a[55] =	0.013526209f;
	a[56] =	0.0070972904f;
	a[57] =	-2.6561373E-10f;
	a[58] =	-0.0053855875f;
	a[59] =	-0.007778209f;
	a[60] =	-0.0071515352f;
	a[61] =	-0.004450674f;
	a[62] =	-0.0010558856f;
	a[63] =	0.0017731993f;
	a[64] =	0.003282209f;
	a[65] =	0.003344623f;
	a[66] =	0.0023420234f;
	a[67] =	9.0173044E-4f;
	a[68] =	-3.8172887E-4f;
	a[69] =	-0.0011395542f;
	a[70] =	-0.001290066f;
	a[71] =	-9.818495E-4f;
	a[72] =	-4.716546E-4f;
	a[73] =	3.1934875E-11f;
	a[74] =	2.8903902E-4f;
	a[75] =	3.6717512E-4f;
	a[76] =	2.9180944E-4f;
	a[77] =	1.5370155E-4f;
	a[78] =	3.0053454E-5f;
	a[79] =	-4.0164417E-5f;
	a[80] =	-5.6314686E-5f;
	a[81] =	-4.0276995E-5f;
	a[82] =	-1.7250879E-5f;


	/*
	 * Kaiser Window FIR Filter
	 * Passband: 0.0 - 1350.0 Hz
	 * modulation freq: 1650Hz
	 * Order: 88
	 * Transition band: 500.0 Hz
	 * Stopband attenuation: 60.0 dB
	 */
	
	b[0] =	-2.081541E-4f;
	b[1] =	-3.5587244E-4f;
	b[2] =	-5.237722E-5f;
	b[3] =	-1.00883444E-4f;
	b[4] =	-8.27162E-4f;
	b[5] =	-7.391658E-4f;
	b[6] =	9.386093E-5f;
	b[7] =	-6.221307E-4f;
	b[8] =	-0.0019506976f;
	b[9] =	-8.508009E-4f;
	b[10] =	2.8596455E-4f;
	b[11] =	-0.002028003f;
	b[12] =	-0.003321186f;
	b[13] =	-2.7830937E-4f;
	b[14] =	2.7148606E-9f;
	b[15] =	-0.004654892f;
	b[16] =	-0.0041854046f;
	b[17] =	0.001115112f;
	b[18] =	-0.0017027275f;
	b[19] =	-0.008291345f;
	b[20] =	-0.0034240147f;
	b[21] =	0.0027767413f;
	b[22] =	-0.005873899f;
	b[23] =	-0.011811939f;
	b[24] =	-2.075215E-8f;
	b[25] =	0.003209243f;
	b[26] =	-0.0131212445f;
	b[27] =	-0.013072912f;
	b[28] =	0.0064319638f;
	b[29] =	1.0081245E-8f;
	b[30] =	-0.023050211f;
	b[31] =	-0.009034872f;
	b[32] =	0.015074444f;
	b[33] =	-0.010180626f;
	b[34] =	-0.034043692f;
	b[35] =	0.004729156f;
	b[36] =	0.024004854f;
	b[37] =	-0.033643555f;
	b[38] =	-0.043601833f;
	b[39] =	0.04075407f;
	b[40] =	0.03076061f;
	b[41] =	-0.10492244f;
	b[42] =	-0.049181364f;
	b[43] =	0.30635652f;
	b[44] =	0.5324795f;
	b[45] =	0.30635652f;
	b[46] =	-0.049181364f;
	b[47] =	-0.10492244f;
	b[48] =	0.03076061f;
	b[49] =	0.04075407f;
	b[50] =	-0.043601833f;
	b[51] =	-0.033643555f;
	b[52] =	0.024004854f;
	b[53] =	0.004729156f;
	b[54] =	-0.034043692f;
	b[55] =	-0.010180626f;
	b[56] =	0.015074444f;
	b[57] =	-0.009034872f;
	b[58] =	-0.023050211f;
	b[59] =	1.0081245E-8f;
	b[60] =	0.0064319638f;
	b[61] =	-0.013072912f;
	b[62] =	-0.0131212445f;
	b[63] =	0.003209243f;
	b[64] =	-2.075215E-8f;
	b[65] =	-0.011811939f;
	b[66] =	-0.005873899f;
	b[67] =	0.0027767413f;
	b[68] =	-0.0034240147f;
	b[69] =	-0.008291345f;
	b[70] =	-0.0017027275f;
	b[71] =	0.001115112f;
	b[72] =	-0.0041854046f;
	b[73] =	-0.004654892f;
	b[74] =	2.7148606E-9f;
	b[75] =	-2.7830937E-4f;
	b[76] =	-0.003321186f;
	b[77] =	-0.002028003f;
	b[78] =	2.8596455E-4f;
	b[79] =	-8.508009E-4f;
	b[80] =	-0.0019506976f;
	b[81] =	-6.221307E-4f;
	b[82] =	9.386093E-5f;
	b[83] =	-7.391658E-4f;
	b[84] =	-8.27162E-4f;
	b[85] =	-1.00883444E-4f;
	b[86] =	-5.237722E-5f;
	b[87] =	-3.5587244E-4f;
	b[88] =	-2.081541E-4f;

	/*
	 * Kaiser Window FIR Filter
	 *
	 * Filter type: Q-filter
	 * Passband: 0.0 - 1350.0 Hz
	 * modulation freq: 1650Hz
	 *  with +90 degree pahse shift
	 * Order: 88
	 * Transition band: 500.0 Hz
	 * Stopband attenuation: 60.0 dB
	 */

	c[0] =	6.767926E-5f;
	c[1] =	-2.1822347E-4f;
	c[2] =	-3.3091355E-4f;
	c[3] =	1.1819744E-4f;
	c[4] =	2.1773627E-9f;
	c[5] =	-8.6602167E-4f;
	c[6] =	-5.9300865E-4f;
	c[7] =	3.814961E-4f;
	c[8] =	-6.342388E-4f;
	c[9] =	-0.00205537f;
	c[10] =	-5.616135E-4f;
	c[11] =	4.8721067E-4f;
	c[12] =	-0.002414588f;
	c[13] =	-0.003538588f;
	c[14] =	-2.7166707E-9f;
	c[15] =	-3.665928E-4f;
	c[16] =	-0.0057645175f;
	c[17] =	-0.004647882f;
	c[18] =	8.681589E-4f;
	c[19] =	-0.0034366683f;
	c[20] =	-0.010545009f;
	c[21] =	-0.0045342376f;
	c[22] =	9.309649E-4f;
	c[23] =	-0.01009504f;
	c[24] =	-0.015788108f;
	c[25] =	-0.0027427748f;
	c[26] =	-0.0020795742f;
	c[27] =	-0.021347176f;
	c[28] =	-0.019808702f;
	c[29] =	-4.1785704E-9f;
	c[30] =	-0.011752444f;
	c[31] =	-0.037658f;
	c[32] =	-0.020762002f;
	c[33] =	8.017756E-4f;
	c[34] =	-0.03406628f;
	c[35] =	-0.060129803f;
	c[36] =	-0.01745214f;
	c[37] =	-0.008082453f;
	c[38] =	-0.08563026f;
	c[39] =	-0.09845453f;
	c[40] =	-0.010001372f;
	c[41] =	-0.06433928f;
	c[42] =	-0.31072536f;
	c[43] =	-0.35893586f;
	c[44] =	0.0f;
	c[45] =	0.35893586f;
	c[46] =	0.31072536f;
	c[47] =	0.06433928f;
	c[48] =	0.010001372f;
	c[49] =	0.09845453f;
	c[50] =	0.08563026f;
	c[51] =	0.008082453f;
	c[52] =	0.01745214f;
	c[53] =	0.060129803f;
	c[54] =	0.03406628f;
	c[55] =	-8.017756E-4f;
	c[56] =	0.020762002f;
	c[57] =	0.037658f;
	c[58] =	0.011752444f;
	c[59] =	4.1785704E-9f;
	c[60] =	0.019808702f;
	c[61] =	0.021347176f;
	c[62] =	0.0020795742f;
	c[63] =	0.0027427748f;
	c[64] =	0.015788108f;
	c[65] =	0.01009504f;
	c[66] =	-9.309649E-4f;
	c[67] =	0.0045342376f;
	c[68] =	0.010545009f;
	c[69] =	0.0034366683f;
	c[70] =	-8.681589E-4f;
	c[71] =	0.004647882f;
	c[72] =	0.0057645175f;
	c[73] =	3.665928E-4f;
	c[74] =	2.7166707E-9f;
	c[75] =	0.003538588f;
	c[76] =	0.002414588f;
	c[77] =	-4.8721067E-4f;
	c[78] =	5.616135E-4f;
	c[79] =	0.00205537f;
	c[80] =	6.342388E-4f;
	c[81] =	-3.814961E-4f;
	c[82] =	5.9300865E-4f;
	c[83] =	8.6602167E-4f;
	c[84] =	-2.1773627E-9f;
	c[85] =	-1.1819744E-4f;
	c[86] =	3.3091355E-4f;
	c[87] =	2.1822347E-4f;
	c[88] =	-6.767926E-5f;
	
	audio_fir = init_fir( 83, a );
	hilbert = init_fir( 89, c );
	delay   = init_fir( 89, b );
	interpolateIQ = init_cfir( 83, a ); 
	nco_enabled = 0 ;
	if( abs(shift_carrier) > 0 ) {	
		m_NcoInc = (2.0 * 3.14159265358979323846)*shift_carrier/SAMPLE_RATE;
		m_OscCos = cos(m_NcoInc);
		m_OscSin = sin(m_NcoInc);

		m_Osc1.re = 1.0;	//initialize unit vector that will get rotated
		m_Osc1.im = 0.0;
		nco_enabled = 1;
	}
}
