#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <librpitx/librpitx.h>

/*
 * Public domain FLEX encoder.
 * Commit: da270b0172a03caf8933eed31d6863ade56566cf
 * Project: https://github.com/Theldus/tinyflex
 */
#include "tinyflex.h"

#define PROGRAM_VERSION "0.1"

// Error message lookup table
static const char *msg_errors[] = {
	"Success",
	"Invalid message buffer",
	"Invalid provided capcode",
	"Invalid provided flex buffer"
};

// FLEX protocol constants
#define FLEX_FIFO_SIZE   12000
#define FLEX_DEVIATION   4800.0f  // FLEX uses 4800 Hz deviation
#define FLEX_SAMPLE_RATE 1600     // Fixed FLEX baud rate

void SendFsk(uint64_t Freq, uint8_t *Message, int Size)
{
	unsigned char *TabSymbol;
	int Sym;
	int i, j;

	fskburst fsktest(Freq - FLEX_DEVIATION, FLEX_SAMPLE_RATE, 
					 FLEX_DEVIATION * 2, 14, FLEX_FIFO_SIZE, 1, 0.0);

	TabSymbol = (unsigned char *) malloc(Size * 8);
	if (TabSymbol == NULL) {
		fprintf(stderr, "Error: Memory allocation failed\n");
		return;
	}

	Sym = 0;
	for (i = 0; i < Size; i++)
	{
		for (j = 7; j >= 0; j--)
		{
			TabSymbol[Sym] = (Message[i] >> j) & 0x1;
			Sym++;
		}
	}

	fsktest.SetSymbols(TabSymbol, Sym);
	fsktest.stop();
	free(TabSymbol);
}

void print_usage(void)
{
	fprintf(stderr,\
			"\nflex -%s\n\
Usage:\nflex [-f Frequency] [-m] [-h]\n\
-f int    central frequency Hz (50 kHz to 1500 MHz, default 916.0 MHz)\n\
-m        enable Mail Drop flag in FLEX message\n\
-h        help (this help)\n\
\nInput format: capcode:message (one per line)\n\
Example: 1234567:Hello World\n\
\n",\
			PROGRAM_VERSION);
}

int main(int argc, char *argv[])
{
	struct tf_message_config config = {0};
	uint8_t flex_buffer[FLEX_BUFFER_SIZE];
	uint8_t *completeTransmission;
	char line[65536];
	size_t completeLength;
	size_t messageLength;
	size_t beforeLength;
	uint64_t SetFrequency;
	char *colon_pos;
	uint64_t capcode;
	char *message;
	size_t msg_len;
	int anyargs;
	int error;
	int a;

	completeTransmission = NULL;
	completeLength       = 0;
	SetFrequency         = 916000000L;
	anyargs              = 1;

	while (1) {
		a = getopt(argc, argv, "f:mh?");

		if (a == -1) {
			if (anyargs)
				break;
			else
				a = '?';
		}
		anyargs = 1;

		switch (a) {
			case 'f':
				SetFrequency = atof(optarg);
				break;

			case 'm':
				config.mail_drop = 1;
				break;

			case 'h':
			default:
				print_usage();
				exit(1);
				break;
		}
	}

	dbg_setlevel(1);

	for (;;) {
		if (fgets(line, sizeof(line), stdin) == NULL)
			break;

		// Find colon separator
		colon_pos = strchr(line, ':');
		if (colon_pos == NULL) {
			fprintf(stderr, 
					"Error: Malformed line - missing ':' separator\n");
			continue;
		}

		*colon_pos = '\0'; // Split the line
		capcode = strtoull(line, NULL, 10);
		message = colon_pos + 1;

		// Remove newline if present
		msg_len = strlen(message);
		if (msg_len > 0 && message[msg_len - 1] == '\n') {
			message[msg_len - 1] = '\0';
		}

		messageLength = tf_encode_flex_message_ex(message, capcode, 
											  flex_buffer, 
											  FLEX_BUFFER_SIZE, &error, &config);

		if (error < 0) {
			if (-error > 0 && -error <= 3) {
				fprintf(stderr, "Error encoding FLEX message: %s\n", 
						msg_errors[-error]);
			} else {
				fprintf(stderr, "Error encoding FLEX message: Unknown error (%d)\n", 
						error);
			}
			continue;
		}

		beforeLength         = completeLength;
		completeLength      += messageLength;
		completeTransmission = (uint8_t *)realloc(completeTransmission, 
												  completeLength);
		if (completeTransmission == NULL) {
			fprintf(stderr, "Error: Memory allocation failed\n");
			return 1;
		}
		
		memcpy(completeTransmission + beforeLength, flex_buffer, 
			   messageLength);
	}
	
	if (completeLength > 0) {
		SendFsk(SetFrequency, completeTransmission, completeLength);
		free(completeTransmission);
	} else {
		fprintf(stderr, "No messages to transmit\n");
	}
	
	return 0;
}
