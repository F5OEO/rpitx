#!/bin/sh

status="0"
INPUT_RTLSDR=434.0
INPUT_GAIN=35
OUTPUT_FREQ=434.0
LAST_ITEM="0 Record"

do_freq_setup()
{

if FREQ=$(whiptail --inputbox "Choose input Frequency (in MHZ) Default is 434MHZ" 8 78 $INPUT_RTLSDR --title "RTL-SDR Receive Frequency" 3>&1 1>&2 2>&3); then
    INPUT_RTLSDR=$FREQ
fi

if GAIN=$(whiptail --inputbox "Choose input Gain (0(AGC) or 1-45)" 8 78 $INPUT_GAIN --title "RTL-SDR Receive Frequency" 3>&1 1>&2 2>&3); then
    INPUT_GAIN=$GAIN
fi

if FREQ=$(whiptail --inputbox "Choose output Frequency (in MHZ) Default is 434MHZ" 8 78 $OUTPUT_FREQ --title "Transmit Frequency" 3>&1 1>&2 2>&3); then
    OUTPUT_FREQ=$FREQ
fi

}

do_stop()
{
	sudo killall rtl_sdr 2>/dev/null
	sudo killall sendiq 2>/dev/null
	sudo killall rtl_fm 2>/dev/null
}
do_status()
{
	 LAST_ITEM="$menuchoice"
	whiptail --title "Processing ""$LAST_ITEM"" on ""$OUTPUT_FREQ""MHZ" --msgbox "Running" 8 78
	do_stop
}

do_freq_setup

 while [ "$status" -eq 0 ]
    do

 menuchoice=$(whiptail --default-item "$LAST_ITEM" --title "Rpitx with RTLSDR" --menu "Choose your test" 20 82 12 \
	"0 Record" "Record spectrum on $INPUT_RTLSDR" \
	"1 Play" "Replay spectrum" \
	"2 Transponder" "Transmit $INPUT_RTLSDR to ""$OUTPUT_FREQ"MHZ \
	"3 Fm->SSB" "Transcode FM $INPUT_RTLSDR to ""$OUTPUT_FREQ"MHZ \
	"4 Set frequency" "Modify frequency (actual $INPUT_RTLSDR Mhz)" \
	3>&2 2>&1 1>&3)

        case "$menuchoice" in
		0\ *) rtl_sdr -s 250000 -g "$INPUT_GAIN" -f "$INPUT_RTLSDR"e6 record.iq >/dev/null 2>/dev/null &
		do_status;;
		1\ *) sudo ./sendiq -s 250000 -f "$OUTPUT_FREQ"e6 -t u8 -i record.iq >/dev/null 2>/dev/null &
		do_status;;
		2\ *) FREQ_IN="$INPUT_RTLSDR"M GAIN="$INPUT_GAIN" FREQ_OUT="$OUTPUT_FREQ"e6 . "$PWD/transponder.sh" >/dev/null 2>/dev/null &
		do_status;;
		3\ *) FREQ_IN="$INPUT_RTLSDR"M GAIN="$INPUT_GAIN" FREQ_OUT="$OUTPUT_FREQ"e6 . "$PWD/fm2ssb.sh" >/dev/null 2>/dev/null &
		do_status;;
		4\ *)
		do_freq_setup;;
		*)	 status=1
		whiptail --title "Bye bye" --msgbox "Thanks for using rpitx!" 8 78
		;;
        esac
    done
