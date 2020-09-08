#!/bin/sh

#TODO using the AM mode from librpitx
echo Need to implement
(while true; do cat sampleaudio.wav; done) | csdr convert_i16_f \
  | csdr gain_ff 4.0 | csdr dsb_fc \
  | sudo ./rpitx -i - -m IQFLOAT -f "$1" -s 48000
