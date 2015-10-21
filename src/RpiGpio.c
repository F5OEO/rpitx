

#include "RpiGpio.h"
#include "mailbox.h"

static volatile unsigned int BCM2708_PERI_BASE;	
static uint32_t dram_phys_base;


char InitGpio()
{
	int   rev, mem, maker, overVolted ;
	//printf("*********** Init GPIO *************\n");
	piBoardId(&model,&rev,&mem,&maker,&overVolted);
	if (model==3) printf(" Model Pi B+\n");
	if(model<3) printf(" Model Pi B\n");
	if(model>=4)
	{
		 printf(" Model Pi 2\n");
		 BCM2708_PERI_BASE = 0x3F000000 ;
		 dram_phys_base   =  0xc0000000;
		 mem_flag         =  0x04;
	}
	else
	{
		 printf("Model Pi 1\n");
		 BCM2708_PERI_BASE = 0x20000000 ;
		 dram_phys_base   =  0x40000000;
		 mem_flag         =  0x0c;
	}

	dma_reg = map_peripheral(DMA_BASE, DMA_LEN);
	pwm_reg = map_peripheral(PWM_BASE, PWM_LEN);
	clk_reg = map_peripheral(CLK_BASE, CLK_LEN);

	pcm_reg =  map_peripheral(PCM_BASE, PCM_LEN); 
	gpio_reg = map_peripheral(GPIO_BASE, GPIO_LEN);
	pad_gpios_reg = map_peripheral(PADS_GPIO, PADS_GPIO_LEN);

	return 1;
	
}

void * map_peripheral(uint32_t base, uint32_t len)
{
	void * vaddr;
	vaddr=mapmem(base,len);
	//printf("Vaddr =%lx \n",vaddr);
	return vaddr;
}


int gpioSetMode(unsigned gpio, unsigned mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpio_reg[reg] = (gpio_reg[reg] & ~(7<<shift)) | (mode<<shift);

   return 0;
}

//*************************************************************************************************************
// ******************  STOLEN FROM WIRING PI ******************************************************************
// ****************** GET PI MODEL WE ARE RUNNING ************************************************************
//*************************************************************************************************************

#ifndef	TRUE
#define	TRUE	(1==1)
#define	FALSE	(1==2)
#endif

static int piModel2 = FALSE ;

const char *piModelNames [7] =
{
  "Unknown",
  "Model A",
  "Model B",
  "Model B+",
  "Compute Module",
  "Model A+",
  "Model 2",	// Quad Core
} ;

const char *piRevisionNames [5] =
{
  "Unknown",
  "1",
  "1.1",
  "1.2",
  "2",
} ;

const char *piMakerNames [5] =
{
  "Unknown",
  "Egoman",
  "Sony",
  "Qusda",
  "MBest",
} ;


// Pi model types and version numbers
//	Intended for the GPIO program Use at your own risk.

#define	PI_MODEL_UNKNOWN	0
#define	PI_MODEL_A		1
#define	PI_MODEL_B		2
#define	PI_MODEL_BP		3
#define	PI_MODEL_CM		4
#define	PI_MODEL_AP		5
#define	PI_MODEL_2		6

#define	PI_VERSION_UNKNOWN	0
#define	PI_VERSION_1		1
#define	PI_VERSION_1_1		2
#define	PI_VERSION_1_2		3
#define	PI_VERSION_2		4

#define	PI_MAKER_UNKNOWN	0
#define	PI_MAKER_EGOMAN		1
#define	PI_MAKER_SONY		2
#define	PI_MAKER_QISDA		3
#define	PI_MAKER_MBEST		4
/*
 * piBoardRev:
 *	Return a number representing the hardware revision of the board.
 *
 *	Revision 1 really means the early Model B's.
 *	Revision 2 is everything else - it covers the B, B+ and CM.
 *		... and the Pi 2 - which is a B+ ++  ...
 *
 *	Seems there are some boards with 0000 in them (mistake in manufacture)
 *	So the distinction between boards that I can see is:
 *	0000 - Error
 *	0001 - Not used 
 *	0002 - Model B,  Rev 1,   256MB, Egoman
 *	0003 - Model B,  Rev 1.1, 256MB, Egoman, Fuses/D14 removed.
 *	0004 - Model B,  Rev 2,   256MB, Sony
 *	0005 - Model B,  Rev 2,   256MB, Qisda
 *	0006 - Model B,  Rev 2,   256MB, Egoman
 *	0007 - Model A,  Rev 2,   256MB, Egoman
 *	0008 - Model A,  Rev 2,   256MB, Sony
 *	0009 - Model A,  Rev 2,   256MB, Qisda
 *	000d - Model B,  Rev 2,   512MB, Egoman
 *	000e - Model B,  Rev 2,   512MB, Sony
 *	000f - Model B,  Rev 2,   512MB, Qisda
 *	0010 - Model B+, Rev 1.2, 512MB, Sony
 *	0011 - Pi CM,    Rev 1.2, 512MB, Sony
 *	0012 - Model A+  Rev 1.2, 256MB, Sony
 *
 *	For the Pi 2:
 *	0010 - Model 2, Rev 1.1, Quad Core, 1GB, Sony
 *
 *	A small thorn is the olde style overvolting - that will add in
 *		1000000
 *
 *	The Pi compute module has an revision of 0011 - since we only check the
 *	last digit, then it's 1, therefore it'll default to not 2 or 3 for a
 *	Rev 1, so will appear as a Rev 2. This is fine for the most part, but
 *	we'll properly detect the Compute Module later and adjust accordingly.
 *
 *********************************************************************************
 */

static void piBoardRevOops (const char *why)
{
  fprintf (stderr, "piBoardRev: Unable to determine board revision from /proc/cpuinfo\n") ;
  fprintf (stderr, " -> %s\n", why) ;
  fprintf (stderr, " ->  You may want to check:\n") ;
  fprintf (stderr, " ->  http://www.raspberrypi.org/phpBB3/viewtopic.php?p=184410#p184410\n") ;
  exit (EXIT_FAILURE) ;
}

int piBoardRev (void)
{
  FILE *cpuFd ;
  char line [120] ;
  char *c ;
  static int  boardRev = -1 ;

  if (boardRev != -1)	// No point checking twice
    return boardRev ;

  if ((cpuFd = fopen ("/proc/cpuinfo", "r")) == NULL)
    piBoardRevOops ("Unable to open /proc/cpuinfo") ;

// Start by looking for the Architecture, then we can look for a B2 revision....

  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Hardware", 8) == 0)
      break ;

  if (strncmp (line, "Hardware", 8) != 0)
    piBoardRevOops ("No \"Hardware\" line") ;

  

// See if it's BCM2708 or BCM2709

  if (strstr (line, "BCM2709") != NULL)
    piModel2 = TRUE ;
  else if (strstr (line, "BCM2708") == NULL)
  {
    fprintf (stderr, "Unable to determine hardware version. I see: %s,\n", line) ;
    fprintf (stderr, " - expecting BCM2708 or BCM2709. Please report this to projects@drogon.net\n") ;
    exit (EXIT_FAILURE) ;
  }

// Now do the rest of it as before

  rewind (cpuFd) ;

  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Revision", 8) == 0)
      break ;

  fclose (cpuFd) ;

  if (strncmp (line, "Revision", 8) != 0)
    piBoardRevOops ("No \"Revision\" line") ;

// Chomp trailing CR/NL

  for (c = &line [strlen (line) - 1] ; (*c == '\n') || (*c == '\r') ; --c)
    *c = 0 ;
  
  //if (wiringPiDebug)
    //printf ("piboardRev: Revision string: %s\n", line) ;

// Scan to first digit

  for (c = line ; *c ; ++c)
    if (isdigit (*c))
      break ;

  if (!isdigit (*c))
    piBoardRevOops ("No numeric revision string") ;

// Make sure its long enough

  if (strlen (c) < 4)
    piBoardRevOops ("Bogus \"Revision\" line (too small)") ;
  
// If you have overvolted the Pi, then it appears that the revision
//	has 100000 added to it!
// The actual condition for it being set is:
//	 (force_turbo || current_limit_override || temp_limit>85) && over_voltage>0

  //if (wiringPiDebug)
    //if (strlen (c) != 4)
      //printf ("piboardRev: This Pi has/is (force_turbo || current_limit_override || temp_limit>85) && over_voltage>0\n") ;

// Isolate  last 4 characters:

  c = c + strlen (c) - 4 ;

  

  if ( (strcmp (c, "0002") == 0) || (strcmp (c, "0003") == 0))
    boardRev = 1 ;
  else
    boardRev = 2 ;

  

  return boardRev ;
}


/*
 * piBoardId:
 *	Do more digging into the board revision string as above, but return
 *	as much details as we can.
 *	This is undocumented and really only intended for the GPIO command.
 *	Use at your own risk!
 *
 * for Pi v2:
 *   [USER:8] [NEW:1] [MEMSIZE:3] [MANUFACTURER:4] [PROCESSOR:4] [TYPE:8] [REV:4]
 *   NEW          23: will be 1 for the new scheme, 0 for the old scheme
 *   MEMSIZE      20: 0=256M 1=512M 2=1G
 *   MANUFACTURER 16: 0=SONY 1=EGOMAN 2=EMBEST
 *   PROCESSOR    12: 0=2835 1=2836
 *   TYPE         04: 0=MODELA 1=MODELB 2=MODELA+ 3=MODELB+ 4=Pi2 MODEL B 5=ALPHA 6=CM
 *   REV          00: 0=REV0 1=REV1 2=REV2
 *********************************************************************************
 */

void piBoardId (int *model, int *rev, int *mem, int *maker, int *overVolted)
{
  FILE *cpuFd ;
  char line [120] ;
  char *c ;

//	Will deal with the properly later on - for now, lets just get it going...
//  unsigned int modelNum ;

  (void)piBoardRev () ;	// Call this first to make sure all's OK. Don't care about the result.

  if ((cpuFd = fopen ("/proc/cpuinfo", "r")) == NULL)
    piBoardRevOops ("Unable to open /proc/cpuinfo") ;

  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Revision", 8) == 0)
      break ;

  fclose (cpuFd) ;

  if (strncmp (line, "Revision", 8) != 0)
    piBoardRevOops ("No \"Revision\" line") ;

// Chomp trailing CR/NL

  for (c = &line [strlen (line) - 1] ; (*c == '\n') || (*c == '\r') ; --c)
    *c = 0 ;
  
  //if (wiringPiDebug)
    //printf ("piboardId: Revision string: %s\n", line) ;

  if (piModel2)
  {

// Scan to the colon

    for (c = line ; *c ; ++c)
      if (*c == ':')
	break ;

    if (*c != ':')
      piBoardRevOops ("Bogus \"Revision\" line") ;

//    modelNum = (unsigned int)strtol (++c, NULL, 16) ; // Hex number with no leading 0x
    
    *model = PI_MODEL_2  ;
    *rev   = PI_VERSION_1_1 ;
    *mem   = 1024 ;
    *maker = PI_MAKER_SONY   ;
  }
  else
  {

// Scan to first digit

    for (c = line ; *c ; ++c)
      if (isdigit (*c))
	break ;

// Make sure its long enough

    if (strlen (c) < 4)
      piBoardRevOops ("Bogus \"Revision\" line") ;

// If longer than 4, we'll assume it's been overvolted

    *overVolted = strlen (c) > 4 ;
  
// Extract last 4 characters:

    c = c + strlen (c) - 4 ;

// Fill out the replys as appropriate

    /**/ if (strcmp (c, "0002") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_1   ; *mem = 256 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "0003") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_1_1 ; *mem = 256 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "0004") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_SONY   ; }
    else if (strcmp (c, "0005") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_QISDA  ; }
    else if (strcmp (c, "0006") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "0007") == 0) { *model = PI_MODEL_A  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "0008") == 0) { *model = PI_MODEL_A  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_SONY ; ; }
    else if (strcmp (c, "0009") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 256 ; *maker = PI_MAKER_QISDA  ; }
    else if (strcmp (c, "000d") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 512 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "000e") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 512 ; *maker = PI_MAKER_SONY   ; }
    else if (strcmp (c, "000f") == 0) { *model = PI_MODEL_B  ; *rev = PI_VERSION_2   ; *mem = 512 ; *maker = PI_MAKER_EGOMAN ; }
    else if (strcmp (c, "0010") == 0) { *model = PI_MODEL_BP ; *rev = PI_VERSION_1_2 ; *mem = 512 ; *maker = PI_MAKER_SONY   ; }
    else if (strcmp (c, "0011") == 0) { *model = PI_MODEL_CM ; *rev = PI_VERSION_1_2 ; *mem = 512 ; *maker = PI_MAKER_SONY   ; }
    else if (strcmp (c, "0012") == 0) { *model = PI_MODEL_AP ; *rev = PI_VERSION_1_2 ; *mem = 256 ; *maker = PI_MAKER_SONY   ; }
    else if (strcmp (c, "0013") == 0) { *model = PI_MODEL_BP ; *rev = PI_VERSION_1_2 ; *mem = 512 ; *maker = PI_MAKER_MBEST  ; }
    else                              { *model = 0           ; *rev = 0              ; *mem =   0 ; *maker = 0 ;               }
  }
}
