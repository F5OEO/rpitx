#ifndef RPITX_H
#define RPITX_H
#include <ctype.h>

int pitx_init(int SampleRate, double TuningFrequency, int* skipSignals);
int pitx_SetTuneFrequencyu(uint32_t Frequency);

#define MODE_IQ 0
#define MODE_RF 1
#define MODE_RFA 2
#define MODE_IQ_FLOAT 3
#define MODE_VFO 4

int pitx_run(
	char Mode,
	int SampleRate,
	float SetFrequency,
	float ppmpll,
	char NoUsePwmFrequency,
	// Wrapper around read to read wav file bytes
	ssize_t (*readWrapper)(void *buffer, size_t count),
	// Wrapper to reset file for looping
	void (*reset)(void),
	int* skipSignals
);


#endif
