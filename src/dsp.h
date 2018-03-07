#ifndef DEF_DSP
#define DEF_DSP

#include "stdint.h"
#include <iostream>
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
class dsp
{
	protected:
	double prev_phase = 0;
	
	double constrainAngle(double x);
	double angleConv(double angle);
	double angleDiff(double a,double b);
	double unwrap(double previousAngle,double newAngle);
	int arctan2(int y, int x);

	public:
	uint32_t samplerate;
	//double phase;
	double amplitude;
	double frequency;

	dsp();
	dsp(uint32_t samplerate);
	void pushsample(liquid_float_complex sample);	

};
#endif
