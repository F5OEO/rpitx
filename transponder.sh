#You need a rtl-sdr dongle in order to run this
echo "transponder FreqIn(Mhz) Gain(0-45)"
rtl_sdr -s 250000 -g "$2" -f "$1" - | buffer | sudo ./sendiq -s 250000 -f 434.0e6 -t u8 -i -

