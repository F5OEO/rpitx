#!/bin/sh

echo "FREQ_IN=value-in_MHz GAIN=value-0_to_45 FREQ_OUT=value-in_MHz fm2ssb"
rtl_fm -f "$FREQ_IN" -s 250k -r 48k -g "$GAIN" - | csdr convert_i16_f \
  | csdr fir_interpolate_cc 2 | csdr dsb_fc \
  | csdr bandpass_fir_fft_cc 0.002 0.06 0.01 | csdr fastagc_ff | buffer \
  | sudo ./sendiq -i /dev/stdin -s 96000 -f "$FREQ_OUT" -t float
# | sox -traw -r48k -es -b16 - -c1 -r 48k -traw -
