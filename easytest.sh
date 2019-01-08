#!/bin/sh

status="0"
OUTPUT_FREQ=434.0
LAST_ITEM="0 Tune"
do_freq_setup()
{

if FREQ=$(whiptail --inputbox "Choose output Frequency (in MHZ) Default is 434MHZ" 8 78 $OUTPUT_FREQ --title "Rpitx transmit Frequency" 3>&1 1>&2 2>&3); then
    OUTPUT_FREQ=$FREQ
fi

}

do_stop_transmit()
{
	sudo killall tune 2>/dev/null
	sudo killall pichirp 2>/dev/null
	sudo killall spectrumpaint 2>/dev/null
	sudo killall pifmrds 2>/dev/null
	sudo killall sendiq 2>/dev/null
	sudo killall pocsag 2>/dev/null
	sudo killall piopera 2>/dev/null
	sudo killall rpitx 2>/dev/null
	sudo killall freedv 2>/dev/null
	sudo killall pisstv 2>/dev/null
	sudo killall csdr 2>/dev/null

	case "$menuchoice" in
			
			0\ *) sudo killall testvfo.sh >/dev/null 2>/dev/null ;;
			1\ *) sudo killall testvfo.sh >/dev/null 2>/dev/null ;;
			2\ *) sudo killall testspectrum.sh >/dev/null 2>/dev/null ;; 
			3\ *) sudo killall snap2spectrum.sh >/dev/null 2>/dev/null ;;
			4\ *) sudo killall  testfmrds.sh >/dev/null 2>/dev/null ;;
			5\ *) sudo killall testnfm.sh >/dev/null 2>/dev/null ;;
			6\ *) sudo killall testssb.sh >/dev/null 2>/dev/null ;;
			7\ *) sudo killall testam.sh >/dev/null 2>/dev/null ;;
			8\ *) sudo killall testfreedv.sh >/dev/null 2>/dev/null ;;
			9\ *) sudo killall testsstv.sh >/dev/null 2>/dev/null ;;
			10\ *) sudo killall testpocsag.sh >/dev/null 2>/dev/null ;;
			11\ *) sudo killall testopera.sh >/dev/null 2>/dev/null ;;
			
	esac		
}

do_status()
{
	 LAST_ITEM="$menuchoice"
	whiptail --title "Transmit ""$LAST_ITEM"" on ""$OUTPUT_FREQ""MHZ" --msgbox "Transmitting" 8 78
	do_stop_transmit
}


do_freq_setup

 while [ "$status" -eq 0 ]
    do

 menuchoice=$(whiptail --default-item "$LAST_ITEM" --title "Rpitx on ""$OUTPUT_FREQ""MHZ" --menu "Range frequency : 50Khz-1Ghz. Choose your test" 20 82 12 \
 	"F Set frequency" "Modify frequency (actual $OUTPUT_FREQ Mhz)" \
	"0 Tune" "Carrier" \
    "1 Chirp" "Moving carrier" \
	"2 Spectrum" "Spectrum painting" \
	"3 RfMyFace" "Snap with Raspicam and RF paint" \
	"4 FmRds" "Broadcast modulation with RDS" \
	"5 NFM" "Narrow band FM" \
	"6 SSB" "Upper Side Bande modulation" \
	"7 AM" "Amplitude Modulation (Poor quality)" \
	"8 FreeDV" "Digital voice mode 800XA" \
	"9 SSTV" "Patern picture" \
	"10 Pocsag" "Pager message" \
    "11 Opera" "Like morse but need Opera decoder" \
 	3>&2 2>&1 1>&3)
		RET=$?
		if [ $RET -eq 1 ]; then
    		exit 0
		elif [ $RET -eq 0 ]; then
			case "$menuchoice" in
			F\ *) do_freq_setup ;;
			0\ *) "./testvfo.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			1\ *) "./testchirp.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			2\ *) "./testspectrum.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			3\ *) "./snap2spectrum.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			4\ *) "./testfmrds.sh" "$OUTPUT_FREQ" >/dev/null 2>/dev/null &
			do_status;;
			5\ *) "./testnfm.sh" "$OUTPUT_FREQ""e3" >/dev/null 2>/dev/null &
			do_status;;
			6\ *) "./testssb.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			7\ *) "./testam.sh" "$OUTPUT_FREQ""e3" >/dev/null 2>/dev/null &
			do_status;;
			8\ *) "./testfreedv.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status;;
			9\ *) "./testsstv.sh" "$OUTPUT_FREQ""e6">/dev/null 2>/dev/null &
			do_status;;
			10\ *) "./testpocsag.sh" "$OUTPUT_FREQ""e6">/dev/null 2>/dev/null &
			do_status;;
			11\ *) "./testopera.sh" "$OUTPUT_FREQ""e6">/dev/null 2>/dev/null &
			do_status;;
			*)	 status=1
			whiptail --title "Bye bye" --msgbox "Thx for using rpitx" 8 78
			;;
			esac
		else
			exit 1
		fi
    done
	exit 0

