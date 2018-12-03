#!/bin/sh

#This is only a Narraw Band FM modulator, for FM broadcast modulation , use PiFMRDS
# Need to use a direct FM modulation with librpitx and not using IQ : TODO
echo "If you need to test broadcast FM, use PiFMRDS"
#(while true; do cat sampleaudio.wav; done) | csdr convert_i16_f | csdr gain_ff 2500 | sudo ./sendiq -i /dev/stdin -s 24000 -f 434e6 -t float 1
(while true; do cat sampleaudio.wav; done) | csdr convert_i16_f \
  | csdr gain_ff 7000 | csdr convert_f_samplerf 20833 \
  | sudo ./rpitx -i- -m RF -f "$1"

