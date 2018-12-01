all: ../pisstv ../piopera ../pifsq ../pichirp ../sendiq ../tune ../freedv ../dvbrf ../pocsag ../spectrumpaint ../pifmrds ../rpitx

CFLAGS	= -Wall -g -O2 -Wno-unused-variable
CXXFLAGS = -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS = librpitx/src/librpitx.a -lm -lrt -lpthread
CCP = c++
CC = cc

CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= librpitx/src/librpitx.a -lm -lrt -lpthread -lsndfile -lliquid

../pissb: ssbgen/test_ssb.c ssbgen/ssb_gen.c ssbgen/liquid_ssb.c librpitx/src/librpitx.a
	$(CC) $(CFLAGS_Pissb) -o ../pissb ssbgen/liquid_ssb.c $(LDFLAGS_Pissb)

../pisstv : sstv/pisstv.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../pisstv sstv/pisstv.cpp  $(LDFLAGS)

../piopera : opera/opera.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../piopera opera/opera.cpp  $(LDFLAGS)

../pifsq : fsq/pifsq.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../pifsq fsq/pifsq.cpp  $(LDFLAGS)

../pichirp : chirp/chirp.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../pichirp chirp/chirp.cpp  $(LDFLAGS)

../sendiq : sendiq.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../sendiq sendiq.cpp  $(LDFLAGS)

../tune : tune.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../tune tune.cpp  $(LDFLAGS)

../freedv : freedv/freedv.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../freedv freedv/freedv.cpp  $(LDFLAGS)

../dvbrf : dvb/dvbrf.cpp dvb/dvbsenco8.s dvb/fec100.c dvb/dvbs2arm_1v30.s librpitx/src/librpitx.a
	$(CC) $(CFLAGS) -c -o dvb/dvbsenco8.o dvb/dvbsenco8.s
	$(CC) $(CFLAGS) -c -o dvb/dvbs2arm_1v30.o dvb/dvbs2arm_1v30.s
	$(CC) $(CFLAGS) -c -o dvb/fec100.o dvb/fec100.c
	$(CCP) $(CXXFLAGS) -o ../dvbrf dvb/dvbrf.cpp dvb/dvbsenco8.o dvb/fec100.o dvb/dvbs2arm_1v30.o $(LDFLAGS)

../pocsag: pocsag/pocsag.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../pocsag pocsag/pocsag.cpp $(LDFLAGS)

../spectrumpaint: spectrumpaint/spectrum.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -o ../spectrumpaint spectrumpaint/spectrum.cpp $(LDFLAGS)

../pifmrds: pifmrds/rds.c pifmrds/waveforms.c pifmrds/pi_fm_rds.cpp pifmrds/fm_mpx.c pifmrds/control_pipe.c librpitx/src/librpitx.a
	$(CC) $(CFLAGS) -c -o pifmrds/rds.o pifmrds/rds.c
	$(CC) $(CFLAGS) -c -o pifmrds/control_pipe.o pifmrds/control_pipe.c
	$(CC) $(CFLAGS) -c -o pifmrds/waveforms.o pifmrds/waveforms.c
	$(CC) $(CFLAGS) -c -o pifmrds/rds_wav.o pifmrds/rds_wav.c
	$(CC) $(CFLAGS) -c -o pifmrds/fm_mpx.o pifmrds/fm_mpx.c
	$(CC) -o pifmrds/rds_wav pifmrds/rds_wav.o pifmrds/rds.o pifmrds/waveforms.o pifmrds/fm_mpx.o -lm -lsndfile
	$(CCP) $(CXXFLAGS) -Wno-write-strings -o ../pifmrds pifmrds/rds.o pifmrds/waveforms.o pifmrds/pi_fm_rds.cpp pifmrds/fm_mpx.o pifmrds/control_pipe.o librpitx/src/librpitx.a -lm -lsndfile -lrt -lpthread

../rpitx: rpitxv1/rpitx.cpp librpitx/src/librpitx.a
	$(CCP) $(CXXFLAGS) -Wno-write-strings -o ../rpitx rpitxv1/rpitx.cpp $(LDFLAGS)

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
	rm -f  ../dvbrf ../sendiq ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 ../pichirp ../tune ../freedv ../piopera ../spectrumpaint ../pocsag ../pifmrds ../rpitx

install: all
	install -m 0755 ../pisstv /usr/bin
	install -m 0755 ../piopera /usr/bin
	install -m 0755 ../pifsq /usr/bin
	install -m 0755 ../pichirp /usr/bin
	install -m 0755 ../sendiq /usr/bin
	install -m 0755 ../tune /usr/bin
	install -m 0755 ../freedv /usr/bin
	install -m 0755 ../dvbrf /usr/bin
	install -m 0755 ../rpitx /usr/bin
