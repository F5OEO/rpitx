#include <unistd.h>
#include <librpitx/librpitx.h>
#include "optparse.h"
#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <time.h>
#include <inttypes.h>

bool running = true;

void print_usage(void)
{
/** Future options :
-g nb : power output
-o nb : GPIO out port
-p freq : frequency of the bit 1 pulse (0 : continuous pulse)
**/
	fprintf(stderr,"sendook : a program to send On-Off-Keying with a Raspberry PI.\n\
usage : sendook [options] \"binary code\"\n\
Options:\n\
-h : this help\n\
-v : verbose (-vv : more verbose)\n\
-d : dry run : do not send anything\n\
-f freq : frequency in Hz (default : 433.92MHz)\n\
-0 nb : duration in microsecond of 0 bit (by default : 500us). Use integer only.\n\
-1 nb : duration in microsecond of 1 bit (by default : 250us)\n\
-r nb : repeat nb times the message (default : 3)\n\
-p nb : pause between each message (default : 1000us=1ms)\n\
\n\
\"binary code\":\n\
  a serie of 0 or 1 char (space allowed and ignored)\n\
\n\
Examples:\n\
  sendook -f 868.3M -0 500 -1 250 -r 3 1010101001010101\n\
    send 0xaa55 three times (with the default pause of 1ms) on 868.3MHz. A 0 is a gap of 500us, a 1 is a pulse of 250us\n\
");

} /* end function print_usage */

static void
terminate(int num)
{
	running = false;
	fprintf(stderr, "Caught signal - Terminating\n");
}

// MIN_DURATION (in us) is based on limitation of the ookbursttiming object
#define MIN_DURATION 10
void FATAL_ERROR(const int exitcode, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "FATAL : ");
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(exitcode);
}

/**
 **/
int main(int argc, char *argv[])
{
	int anyargs = 1;
	int a;
	uint64_t Freq = 433920000;
	uint64_t bit0duration = 500; // in microsecond
	uint64_t bit1duration = 500;
	int nbrepeat = 3;
	int pause = 1000; // in us
	int dryrun = 0; // if 1 : hte message is not really transmitted
	char *bits = NULL;
	
	for (int i = 0; i < 64; i++)
	{
		struct sigaction sa;
		std::memset(&sa, 0, sizeof(sa));
		sa.sa_handler = terminate;
		sigaction(i, &sa, NULL);
	}
	while(1)
	{
		a = getopt(argc, argv, "f:0:1:r:p:hvd");
		if(a == -1)
		{
			if(anyargs) break;
			else a='h'; //print usage and exit
		}
		anyargs = 1;
		switch(a)
		{
			case 'f': // Frequency
				Freq = atouint32_metric(optarg, "Error with -f : ");
				break;
			case '0': // bit 0 duration
				bit0duration = atouint32_metric(optarg, "Error with -0 : ");
				break;
			case '1': // bit 0 duration
				bit1duration = atouint32_metric(optarg, "Error with -1 : ");
				break;
			case 'r':
				nbrepeat = atoi(optarg);
				break;
			case 'p':
				pause = atoi(optarg);
				break;
			case 'h' :
				print_usage();
				exit(0);
				break;
			case 'v': // verbose
				dbg_setlevel(dbg_getlevel() + 1);
				break;
			case 'd': // Dry run
				dryrun = 1;
				break;
			case -1:
				break;
			default:
				print_usage();
				exit(1);
				break;
		}/* end switch a */
	}/* end while getopt() */
	if (optind >= argc) {
		FATAL_ERROR(-2, "Missing bit message.\n");
	}
	bits = argv[optind];
	printf("Frequency set to : %" PRIu64 "Hz \n", Freq);
	printf("Bit duration 0 : %" PRIu64 "us ; 1 : %" PRIu64 "us\n",
		bit0duration, bit1duration);
	printf("Send message %d times with a pause of %dus\n", nbrepeat, pause);
	if (dryrun) 
		printf("Dry run mode enabled : no message will be sent\n");
	dbg_printf(1, "Verbose mode enabled, level %d.\n", dbg_getlevel());
	// Simplify the message to send
	int computed_duration = 0; // in us	
	int nbbits = 0;
	for(size_t i = 0; i < strlen(bits); i++)
	{
		char c = bits[i];
		if (c == '0')
		{
			nbbits ++;
			computed_duration += bit0duration;
		} else if (c == '1')
		{
			nbbits ++;
			computed_duration += bit1duration;
		}
		// any other char is ignored (it allows to speparate nibble with a space for example)
		// improvement : allow "." and "-" or "i" and "a" to create a MORSE sender
	}
	dbg_printf(1, "Send %d bits, with a total duration of %d us.\n", nbbits, computed_duration);
	if (computed_duration == 0 || nbbits == 0)
	{
		FATAL_ERROR(-2, "Message duration or number of bits invalid.\n");
	}
	if (bit0duration < MIN_DURATION || bit1duration < MIN_DURATION)
	{
		FATAL_ERROR(-2, "Currently, sendook support only bit longer than 10us.\n");
	}
	
	// Prepare the message
	ookbursttiming ooksender(Freq, computed_duration);
	ookbursttiming::SampleOOKTiming Message[nbbits];
	for(size_t i = 0; i < strlen(bits); i++)
	{
		char c = bits[i];
		if (c == '0')
		{
			Message[i].value = 0; 
			Message[i].duration = bit0duration;
		} else if (c == '1')
		{
			Message[i].value = 1; 
			Message[i].duration = bit1duration;
		}
	}
	
	// Send the message
	for (int i = 0; i < nbrepeat; i++)
	{
		if (!dryrun)
			ooksender.SendMessage(Message, nbbits); 
		else
		{
			printf("Simulating SendMessage of %d bits\n", nbbits);
			usleep(computed_duration);
		}
		if(!running)
			break;
		if (i<nbrepeat-1)
		{
			usleep(pause);
		}
		if(!running)
			break;
	}
	printf("Message successfuly transmitted\n");
	return 0;
}
