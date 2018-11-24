echo fm2ssb.sh freq gain
rtl_fm -f $1 -s 250k -r 48k -g $2 -  | csdr convert_i16_f | csdr fir_interpolate_cc 2| csdr dsb_fc | csdr bandpass_fir_fft_cc 0.002 0.06 0.01 | csdr fastagc_ff | buffer | sudo ./sendiq -i /dev/stdin -s 96000 -f $3 -t float
# | sox -traw -r48k -es -b16 - -c1 -r 48k -traw -