#!/bin/bash
status="0"
INPUT_RTLSDR=434.0
INPUT_GAIN=0

do_freq_setup()
{

FREQ=$(whiptail --inputbox "Choose input Frequency (in MHZ) Default is 100MHZ" 8 78 $INPUT_RTLSDR --title "RTL-SDR Receive Frequency" 3>&1 1>&2 2>&3)
if [ $? -eq 0 ]; then
    INPUT_RTLSDR=$FREQ
fi

GAIN=$(whiptail --inputbox "Choose input Gain (0(AGC) or 1-45)" 8 78 $INPUT_GAIN --title "RTL-SDR Receive Frequency" 3>&1 1>&2 2>&3)
if [ $? -eq 0 ]; then
    INPUT_GAIN=$GAIN
fi

}

do_freq_setup

 while [ "$status" -eq 0 ] 
    do

 menuchoice=$(whiptail --title "Rpitx with RTLSDR" --menu "Choose your test, ctrl^c to end a test" 20 82 12 \
	"0 Record" "Record spectrum on $INPUT_RTLSDR" \
	"1 Play" "Replay spectrum" \
	"2 Transponder" "Transmit $INPUT_RTLSDR to 434MHZ" \
	"3 Fm->SSB" "Transcode FM $INPUT_RTLSDR to 434MHZ" \
	"4 Set frequency" "Modify frequency (actual $INPUT_RTLSDR Mhz)" \
	3>&2 2>&1 1>&3)

        case "$menuchoice" in
		0\ *) rtl_sdr -s 250000 -g "$INPUT_GAIN" -f "$INPUT_RTLSDR"M record.iq >/dev/null 2>/dev/null  ;;
	    1\ *) sudo ./sendiq -s 250000 -f "$INPUT_RTLSDR"e6 -t u8 -i record.iq >/dev/null 2>/dev/null ;;
		2\ *) source $"$PWD/transponder.sh" "$INPUT_RTLSDR"M $INPUT_GAIN  ;;
		3\ *) source $"$PWD/fm2ssb.sh" "$INPUT_RTLSDR"M $INPUT_GAIN >/dev/null 2>/dev/null ;;
		4\ *) do_freq_setup ;;
		   *)	 status=1;;
        esac
       
    done
exit
