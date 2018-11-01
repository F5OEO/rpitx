#all: ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77
all: ../pisstv ../piopera ../pifsq ../pichirp ../sendiq ../tune ../freedv ../dvbrf ../pocsag ../spectrumpaint

CFLAGS	= -Wall -g -O2 -Wno-unused-variable 
LDFLAGS	= librpitx/src/librpitx.a -lm -lrt -lpthread 
CCP= g++
CC= gcc

../rpitx: RpiTx.cpp librpitx/src/librpitx.h
		$(CCP) $(CFLAGS) -o ../rpitx  RpiTx.c $(LDFLAGS) 
		
CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= librpitx/src/librpitx.a -lm -lrt -lpthread -lsndfile -lliquid

../pissb: ssbgen/test_ssb.c ssbgen/ssb_gen.c ssbgen/liquid_ssb.c
		$(CC) $(CFLAGS_Pissb) -o ../pissb ssbgen/liquid_ssb.c $(LDFLAGS_Pissb) 

../pisstv : sstv/pisstv.cpp 
	$(CCP) $(CFLAGS) -o ../pisstv sstv/pisstv.cpp  $(LDFLAGS) 


../piopera : opera/opera.cpp 
	$(CCP) $(CFLAGS) -o ../piopera opera/opera.cpp  $(LDFLAGS)


../pifsq : fsq/pifsq.cpp 
	$(CCP) $(CFLAGS) -o ../pifsq fsq/pifsq.cpp  $(LDFLAGS) 

../pichirp : chirp/chirp.cpp 
	$(CCP) $(CFLAGS) -o ../pichirp chirp/chirp.cpp  $(LDFLAGS) 

../sendiq : sendiq.cpp librpitx/src/librpitx.a
	$(CCP) $(CFLAGS) -o ../sendiq sendiq.cpp  $(LDFLAGS) 

../tune : tune.cpp 
	$(CCP) $(CFLAGS) -o ../tune tune.cpp  $(LDFLAGS) 

../freedv : freedv/freedv.cpp 
	$(CCP) $(CFLAGS) -o ../freedv freedv/freedv.cpp  $(LDFLAGS) 	

../dvbrf : dvb/dvbrf.cpp dvb/dvbsenco8.s dvb/fec100.c dvb/dvbs2arm_1v30.s librpitx/src/librpitx.a
	$(CC) $(CFLAGS) -c -o dvb/dvbsenco8.o dvb/dvbsenco8.s
	$(CC) $(CFLAGS) -c -o dvb/dvbs2arm_1v30.o dvb/dvbs2arm_1v30.s
	$(CC) $(CFLAGS) -c -o dvb/fec100.o dvb/fec100.c
	$(CCP) $(CFLAGS) -o ../dvbrf dvb/dvbrf.cpp dvb/dvbsenco8.o dvb/fec100.o dvb/dvbs2arm_1v30.o $(LDFLAGS) 	

../pocsag: pocsag/pocsag.cpp librpitx/src/librpitx.a
	$(CCP) $(CFLAGS) -o ../pocsag pocsag/pocsag.cpp $(LDFLAGS) 	

../spectrumpaint: spectrumpaint/spectrum.cpp librpitx/src/librpitx.a
	$(CCP) $(CFLAGS) -o ../spectrumpaint spectrumpaint/spectrum.cpp $(LDFLAGS) 	

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
	
	rm -f  ../dvbrf ../rpitx ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 ../pichirp ../tune ../freedv RpiTx.o mailbox.o RpiGpio.o RpiDma.o

install: all
	install -m 0755 ../pisstv /usr/bin
	install -m 0755 ../piopera /usr/bin
	install -m 0755 ../pifsq /usr/bin
	install -m 0755 ../pichirp /usr/bin
	install -m 0755 ../sendiq /usr/bin
	install -m 0755 ../tune /usr/bin
	install -m 0755 ../freedv /usr/bin
	install -m 0755 ../dvbrf /usr/bin
	
