#You need a rtl-sdr dongle in order to run this
rtl_sdr -s 250000 -g 40 -f 107700000 - | sudo ./sendiq -s 250000 -f 434.0e6 -t u8 -i -

