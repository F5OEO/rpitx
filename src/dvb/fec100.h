typedef unsigned char 	uchar ;		// 8 bit
typedef unsigned short	uint16 ;	// 16 bit
typedef unsigned int	uint ;		// 32 bits

void viterbi_init(int SetFEC);
uint16 viterbi (uchar *in, uchar *out);

