all: ../pisstv ../piopera ../pifsq ../pichirp ../sendiq ../tune ../freedv ../pocsag ../spectrumpaint ../pifmrds ../rpitx ../corel8 ../pift8 ../sendook ../morse ../foxhunt ../pirtty ../dvbrf

CFLAGS	?= -Wall -g -O2 -Wno-unused-variable
CXXFLAGS ?= -std=c++11 -Wall -g -O2 -Wno-unused-variable
LDFLAGS ?= -L/opt/vc/lib
LDFLAGS += -lrpitx -lm -lrt -lpthread
CXX ?= c++
CC ?= cc
INSTALL_DIR ?= /usr/local/bin

CFLAGS_Pissb	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pissb	= $(LDFLAGS) -lsndfile -lliquid

../pissb: ssbgen/test_ssb.c ssbgen/ssb_gen.c ssbgen/liquid_ssb.c 
	$(CC) $(CFLAGS_Pissb) -o ../pissb ssbgen/liquid_ssb.c $(LDFLAGS_Pissb)

../pisstv : sstv/pisstv.cpp 
	$(CXX) $(CXXFLAGS) -o ../pisstv sstv/pisstv.cpp  $(LDFLAGS)
	
../foxhunt : foxhunt/foxhunt.cpp
	$(CXX) $(CXXFLAGS) -o ../foxhunt foxhunt/foxhunt.cpp  $(LDFLAGS)
	
	
../pirtty : pirtty/pirtty.cpp
	$(CXX) $(CXXFLAGS) -o ../pirtty pirtty/pirtty.cpp  $(LDFLAGS)

../piopera : opera/opera.cpp 
	$(CXX) $(CXXFLAGS) -o ../piopera opera/opera.cpp  $(LDFLAGS)

../pifsq : fsq/pifsq.cpp 
	$(CXX) $(CXXFLAGS) -o ../pifsq fsq/pifsq.cpp  $(LDFLAGS)

../pichirp : chirp/chirp.cpp 
	$(CXX) $(CXXFLAGS) -o ../pichirp chirp/chirp.cpp $(LDFLAGS) 

../morse : morse/morse.cpp 
	$(CXX) $(CXXFLAGS) -o ../morse morse/morse.cpp  $(LDFLAGS)

../sendiq : sendiq.cpp 
	$(CXX) $(CXXFLAGS) -o ../sendiq sendiq.cpp  $(LDFLAGS)

../tune : tune.cpp 
	$(CXX) $(CXXFLAGS) -o ../tune tune.cpp  $(LDFLAGS)

../freedv : freedv/freedv.cpp 
	$(CXX) $(CXXFLAGS) -o ../freedv freedv/freedv.cpp  $(LDFLAGS)

../dvbrf : dvb/dvbrf.cpp dvb/dvbsenco8.s dvb/fec100.c dvb/dvbs2arm_1v30.s 
	$(CC) $(CFLAGS) -c -o dvb/dvbsenco8.o dvb/dvbsenco8.s
	$(CC) $(CFLAGS) -c -o dvb/dvbs2arm_1v30.o dvb/dvbs2arm_1v30.s
	$(CC) $(CFLAGS) -c -o dvb/fec100.o dvb/fec100.c
	$(CXX) $(CXXFLAGS) -o ../dvbrf dvb/dvbrf.cpp dvb/dvbsenco8.o dvb/fec100.o dvb/dvbs2arm_1v30.o $(LDFLAGS)

../pocsag: pocsag/pocsag.cpp 
	$(CXX) $(CXXFLAGS) -o ../pocsag pocsag/pocsag.cpp $(LDFLAGS)

../spectrumpaint: spectrumpaint/spectrum.cpp 
	$(CXX) $(CXXFLAGS) -o ../spectrumpaint spectrumpaint/spectrum.cpp $(LDFLAGS)

../pifmrds: pifmrds/rds.c pifmrds/waveforms.c pifmrds/pi_fm_rds.cpp pifmrds/fm_mpx.c pifmrds/control_pipe.c 
	$(CC) $(CFLAGS) -c -o pifmrds/rds.o pifmrds/rds.c
	$(CC) $(CFLAGS) -c -o pifmrds/control_pipe.o pifmrds/control_pipe.c
	$(CC) $(CFLAGS) -c -o pifmrds/waveforms.o pifmrds/waveforms.c
	$(CC) $(CFLAGS) -c -o pifmrds/rds_wav.o pifmrds/rds_wav.c
	$(CC) $(CFLAGS) -c -o pifmrds/fm_mpx.o pifmrds/fm_mpx.c
	$(CC) -o pifmrds/rds_wav pifmrds/rds_wav.o pifmrds/rds.o pifmrds/waveforms.o pifmrds/fm_mpx.o -lm -lsndfile
	$(CXX) $(CXXFLAGS) -Wno-write-strings -o ../pifmrds pifmrds/rds.o pifmrds/waveforms.o pifmrds/pi_fm_rds.cpp pifmrds/fm_mpx.o pifmrds/control_pipe.o -lm -lsndfile -lrt -lpthread -L/opt/vc/lib -lrpitx

../rpitx: rpitxv1/rpitx.cpp 
	$(CXX) $(CXXFLAGS) -Wno-write-strings -o ../rpitx rpitxv1/rpitx.cpp $(LDFLAGS)

../corel8: corel8/corel8.cpp corel8/costas8.h 
	$(CXX) $(CXXFLAGS) -Wno-write-strings -o ../corel8 corel8/corel8.cpp $(LDFLAGS)

../pift8 : pift8/pift8.cpp  
	$(CXX) $(CXXFLAGS) -Wno-write-strings -o ../pift8 pift8/pift8.cpp -lft8 $(LDFLAGS) 

../sendook: ook/sendook.cpp ook/optparse.c 
	$(CXX) $(CXXFLAGS) -Wno-write-strings -o ../sendook ook/sendook.cpp ook/optparse.c $(LDFLAGS)

CFLAGS_Pifm	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Pifm	= $(LDFLAGS) -lsndfile
../pifm : ../fm/pifm.c
	$(CC) $(CFLAGS_Pifm) -o ../pifm ../fm/pifm.c  $(LDFLAGS_Pifm)

CFLAGS_Piam	= -Wall -g -O2 -Wno-unused-variable
LDFLAGS_Piam	= $(LDFLAGS_Pifm)
../piam : ../am/piam.c
	$(CC) $(CFLAGS_Piam) -o ../piam ../am/piam.c  $(LDFLAGS_Piam)

CFLAGS_Pidcf77	= -Wall -g -O2 -Wno-unused-variable
../pidcf77 : ../dcf77/pidcf77.c
	$(CC) $(CFLAGS_Piam) -o ../pidcf77 ../dcf77/pidcf77.c  $(LDFLAGS)
clean:
	rm -f  ../dvbrf ../sendiq ../pissb ../pisstv ../pifsq ../pifm ../piam ../pidcf77 ../pichirp ../tune ../freedv ../piopera ../spectrumpaint ../pocsag ../pifmrds ../rpitx ../sendook ../foxhunt ../morse ../pift8

install: all
	install -m 0755 ../pisstv $(INSTALL_DIR)
	install -m 0755 ../foxhunt $(INSTALL_DIR)
	install -m 0755 ../pirtty $(INSTALL_DIR)
	install -m 0755 ../piopera $(INSTALL_DIR)
	install -m 0755 ../pifsq $(INSTALL_DIR)
	install -m 0755 ../pichirp $(INSTALL_DIR)
	install -m 0755 ../sendiq $(INSTALL_DIR)
	install -m 0755 ../tune $(INSTALL_DIR)
	install -m 0755 ../freedv $(INSTALL_DIR)
	install -m 0755 ../rpitx $(INSTALL_DIR)
	install -m 0755 ../pift8 $(INSTALL_DIR)
	install -m 0755 ../sendook $(INSTALL_DIR)
	install -m 0755 ../dvbrf $(INSTALL_DIR)
	install -m 0755 ../pifmrds $(INSTALL_DIR)
	install -m 0755 ../pocsag $(INSTALL_DIR)
	install -m 0755 ../pissb $(INSTALL_DIR)
	install -m 0755 ../piam $(INSTALL_DIR)
	install -m 0755 ../pidcf77 $(INSTALL_DIR)
	install -m 0755 ../spectrumpaint $(INSTALL_DIR)
	install -m 0755 ../morse $(INSTALL_DIR)
