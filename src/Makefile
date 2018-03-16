#all: ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 ../piopera
all:  librpitx v2rpitx ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 ../piopera

#CFLAGS	= -Wall -g -O2 -D DIGITHIN
CFLAGS	= -Wall -g -O2 -Wno-unused-variable 
LDFLAGS	= -lm -lrt -lpthread 


../rpitx: RpiGpio.c RpiTx.c  mailbox.c RpiDma.c raspberry_pi_revision.c calibrationpi2.h calibrationpizero.h calibrationpi3.h
		$(CC) $(CFLAGS) -o ../rpitx  RpiTx.c RpiGpio.c mailbox.c RpiDma.c raspberry_pi_revision.c $(LDFLAGS) 


CFLAGS	= -Wall -g -O3 -Wno-unused-variable 
LDFLAGS	= -lm -lrt -lpthread -lliquid
CCP = g++ 
CC = gcc 

librpitx: librpitx.h gpio.h gpio.cpp dma.h dma.cpp mailbox.c raspberry_pi_revision.c fmdmasync.h fmdmasync.cpp ngfmdmasync.h ngfmdmasync.cpp dsp.h dsp.cpp iqdmasync.h iqdmasync.cpp serialdmasync.h serialdmasync.cpp phasedmasync.h phasedmasync.cpp fskburst.h fskburst.cpp
	$(CC) $(CFLAGS) -c -o mailbox.o mailbox.c
	$(CC) $(CFLAGS) -c -o raspberry_pi_revision.o raspberry_pi_revision.c
	$(CCP) $(CFLAGS) -c dsp.cpp iqdmasync.cpp ngfmdmasync.cpp fmdmasync.cpp  dma.cpp gpio.cpp mailbox.o raspberry_pi_revision.o v2rpitx.cpp serialdmasync.cpp phasedmasync.cpp amdmasync.h amdmasync.cpp fskburst.cpp
	$(AR) rc librpitx.a dsp.o iqdmasync.o ngfmdmasync.o fmdmasync.o  dma.o gpio.o mailbox.o raspberry_pi_revision.o  serialdmasync.o phasedmasync.o amdmasync.o fskburst.o

v2rpitx: librpitx.h librpitx.a
	
	$(CCP) $(CFLAGS) -o v2rpitx v2rpitx.cpp librpitx.a $(LDFLAGS) 


		
CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= -lm -lrt -lpthread -lsndfile

../pissb: ../ssbgen/test_ssb.c ../ssbgen/ssb_gen.c
		$(CC) $(CFLAGS_Pissb) -o ../pissb ../ssbgen/ssb_gen.c ../ssbgen/test_ssb.c $(LDFLAGS_Pissb) 

CFLAGS_Pisstv	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pisstv	= -lm -lrt -lpthread 
../pisstv : ../sstv/v2pisstv.cpp 
	$(CCP) $(CFLAGS_Pisstv) -o ../pisstv ../sstv/v2pisstv.cpp ../src/librpitx.a  $(LDFLAGS_Pisstv) 

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

CFLAGS_Pidcf77	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pidcf77	= -lm -lrt -lpthread
../pidcf77 : ../dcf77/pidcf77.c 
	$(CC) $(CFLAGS_Piam) -o ../pidcf77 ../dcf77/pidcf77.c  $(LDFLAGS_Piam) 

CFLAGS_Piopera	= -g -O2 -Wno-unused-variable
LDFLAGS_Piopera	= -lm 
../piopera : ../opera/opera.c
	$(CC) $(CFLAGS_Piopera) -o ../piopera ../opera/opera.c  $(LDFLAGS_Piopera) 

clean:
	
	rm -f  ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 v2rpitx RpiTx.o mailbox.o RpiGpio.o RpiDma.o

install: all
	install -m 0755 ../pisstv /usr/bin
	install -m 0755 ../pifm /usr/bin
	install -m 0755 ../piam /usr/bin
	install -m 0755 ../pissb /usr/bin
	install -m 0755 ../pifsq /usr/bin
	install -m 0755 ../rpitx /usr/bin
	install -m 0755 ../pidcf77 /usr/bin
	install -m 0755 ../piopera /usr/bin
	cp dt-blob.bin /boot/
	$(info !!! You should reboot if it is the first installation !!!)
