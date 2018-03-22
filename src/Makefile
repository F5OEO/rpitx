#all: ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77
all: ../pisstv ../piopera ../pifsq

CFLAGS	= -Wall -g -O2 -Wno-unused-variable 
LDFLAGS	= librpitx/src/librpitx.a -lm -lrt -lpthread 
CC= g++

../rpitx: RpiTx.cpp librpitx/src/librpitx.h
		$(CC) $(CFLAGS) -o ../rpitx  RpiTx.c $(LDFLAGS) 
		
CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= librpitx/src/librpitx.a -lm -lrt -lpthread -lsndfile

../pissb: ../ssbgen/test_ssb.c ../ssbgen/ssb_gen.c
		$(CC) $(CFLAGS_Pissb) -o ../pissb ../ssbgen/ssb_gen.c ../ssbgen/test_ssb.c $(LDFLAGS_Pissb) 

CFLAGS_Pisstv	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pisstv	= librpitx/src/librpitx.a -lm -lrt -lpthread 
../pisstv : sstv/pisstv.cpp 
	$(CC) $(CFLAGS_Pisstv) -o ../pisstv sstv/pisstv.cpp  $(LDFLAGS_Pisstv) 



../piopera : opera/opera.cpp 
	$(CC) $(CFLAGS) -o ../piopera opera/opera.cpp  $(LDFLAGS)


../pifsq : fsq/pifsq.cpp 
	$(CC) $(CFLAGS) -o ../pifsq fsq/pifsq.cpp  $(LDFLAGS) 

CFLAGS_Pifm	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pifm	= librpitx/src/librpitx.a -lm -lrt -lpthread -lsndfile
../pifm : ../fm/pifm.c 
	$(CC) $(CFLAGS_Pifm) -o ../pifm ../fm/pifm.c  $(LDFLAGS_Pifm) 

CFLAGS_Piam	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Piam	= librpitx/src/librpitx.a -lm -lrt -lpthread -lsndfile
../piam : ../am/piam.c 
	$(CC) $(CFLAGS_Piam) -o ../piam ../am/piam.c  $(LDFLAGS_Piam) 

CFLAGS_Pidcf77	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pidcf77	= librpitx/src/librpitx.a -lm -lrt -lpthread
../pidcf77 : ../dcf77/pidcf77.c 
	$(CC) $(CFLAGS_Piam) -o ../pidcf77 ../dcf77/pidcf77.c  $(LDFLAGS_Piam) 
clean:
	
	rm -f  ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 RpiTx.o mailbox.o RpiGpio.o RpiDma.o

install: all
	install -m 0755 ../pisstv /usr/bin
	install -m 0755 ../pifm /usr/bin
	install -m 0755 ../piam /usr/bin
	install -m 0755 ../pissb /usr/bin
	install -m 0755 ../pifsq /usr/bin
	install -m 0755 ../rpitx /usr/bin
	install -m 0755 ../pidcf77 /usr/bin
	
