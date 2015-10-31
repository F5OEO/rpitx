all: ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam

#CFLAGS	= -Wall -g -O2 -D DIGITHIN
CFLAGS	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS	= -lm -lrt -lpthread 


../rpitx: RpiGpio.c RpiTx.c  mailbox.c RpiDma.c 
		$(CC) $(CFLAGS) -o ../rpitx  RpiTx.c RpiGpio.c mailbox.c RpiDma.c $(LDFLAGS) 
		
CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= -lm -lrt -lpthread -lsndfile

../pissb: ../ssbgen/test_ssb.c ../ssbgen/ssb_gen.c
		$(CC) $(CFLAGS_Pissb) -o ../pissb ../ssbgen/ssb_gen.c ../ssbgen/test_ssb.c $(LDFLAGS_Pissb) 

CFLAGS_Pisstv	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pisstv	= -lm -lrt -lpthread 
../pisstv : ../sstv/pisstv.c 
	$(CC) $(CFLAGS_Pisstv) -o ../pisstv ../sstv/pisstv.c  $(LDFLAGS_Pisstv) 

CFLAGS_Pifsq	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pifsq	= -lm -lrt -lpthread 
../pifsq : ../fsq/pifsq.c 
	$(CC) $(CFLAGS_Pisfq) -o ../pifsq ../fsq/pifsq.c  $(LDFLAGS_Pisfq) 

CFLAGS_Pifm	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pifm	= -lm -lrt -lpthread -lsndfile
../pifm : ../fm/pifm.c 
	$(CC) $(CFLAGS_Pifm) -o ../pifm ../fm/pifm.c  $(LDFLAGS_Pifm) 

CFLAGS_Piam	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Piam	= -lm -lrt -lpthread -lsndfile
../piam : ../am/piam.c 
	$(CC) $(CFLAGS_Piam) -o ../piam ../am/piam.c  $(LDFLAGS_Piam) 
clean:
	
	rm -f  ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam RpiTx.o mailbox.o RpiGpio.o RpiDma.o

install: all
	install -m 0755 ../pisstv /usr/bin
	install -m 0755 ../pifm /usr/bin
	install -m 0755 ../piam /usr/bin
	install -m 0755 ../pissb /usr/bin
	install -m 0755 ../pifsq /usr/bin
	install -m 0755 ../rpitx /usr/bin
