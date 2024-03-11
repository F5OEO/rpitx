#!/bin/sh
abort_action=0

OUTPUT_FREQ=434.0
DEFAUL_JPG_PICTURE_LOC=src/resources/SAMPLE_IMAGE.jpg
DEFAULT_WAV_FILE_MONO_LOC=src/resources/SAMPLE_MONO_AUDIO.wav
DEFAULT_WAV_FILE_STEREO_LOC=src/resources/SAMPLE_STEREO_AUDIO.wav
DEFAULT_RF_FREEDV_FILE_LOC=src/resources/SAMPLE_FREEDV.rf
DEFAULT_POCSAG_MESSAGE="1:YOURCALL\n2: Hello world"
DEFAULT_OPERA_CALLSIGN="F5OEO"
DEFAULT_RTTY_MESSAGE="HELLO WORLD FROM RPITX"
LAST_ITEM="0 Tune"

do_check_file_existance() 
{

	readlink -e $1 > /dev/null
	if [ $? -eq 1 ]; then
    	whiptail --title "Error!" --msgbox "The file does not exist!" 8 78
		return 1
	fi
	return 0

}

do_freq_setup()
{

if FREQ=$(whiptail --inputbox "Enter output Frequency (in MHz). Current is $OUTPUT_FREQ MHz" 8 78 $OUTPUT_FREQ --title "Rpitx transmit Frequency" 3>&1 1>&2 2>&3); then
	OUTPUT_FREQ=$FREQ
fi

}

do_file_choose()
{

LAST_ITEM="$menuchoice"
if FILE_LOC=$(whiptail --inputbox "Enter $1 file location. Default is $2" 8 78 $2 --title "Select a file to transmit" 3>&1 1>&2 2>&3); then
	do_check_file_existance "$FILE_LOC"
	abort_action=$?
else
	abort_action=1
fi

}

do_enter_message()
{

LAST_ITEM="$menuchoice"
if MESSAGE=$(whiptail --inputbox "Type custom $1 message:" 8 78 "$2" --title "Enter message to transmit" 3>&1 1>&2 2>&3);  then
	abort_action=0
	if [ -z "$MESSAGE" ]; then
    	whiptail --title "Error!" --msgbox "Empty message!" 8 78
		abort_action=1
	fi
else
	abort_action=1
fi

}

do_enter_callsign()
{

LAST_ITEM="$menuchoice"
if CALLSIGN=$(whiptail --inputbox "Type callsign:" 8 78 "$DEFAULT_OPERA_CALLSIGN" --title "Enter callsign to transmit" 3>&1 1>&2 2>&3);  then
	abort_action=0
	if [ -z "$CALLSIGN" ]; then
    	whiptail --title "Error!" --msgbox "Empty callsign!" 8 78
		abort_action=1
	fi
else
	abort_action=1
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
	sudo killall pirtty 2>/dev/null

	case "$menuchoice" in
			
			0\ *) sudo killall testvfo.sh >/dev/null 2>/dev/null ;;
			1\ *) sudo killall testvfo.sh >/dev/null 2>/dev/null ;;
			2\ *) sudo killall testspectrum.sh >/dev/null 2>/dev/null ;; 
			3\ *) sudo killall snap2spectrum.sh >/dev/null 2>/dev/null ;;
			4\ *) sudo killall testfmrds.sh >/dev/null 2>/dev/null ;;
			5\ *) sudo killall testnfm.sh >/dev/null 2>/dev/null ;;
			6\ *) sudo killall testssb.sh >/dev/null 2>/dev/null ;;
			7\ *) sudo killall testam.sh >/dev/null 2>/dev/null ;;
			8\ *) sudo killall testfreedv.sh >/dev/null 2>/dev/null ;;
			9\ *) sudo killall testsstv.sh >/dev/null 2>/dev/null ;;
			10\ *) sudo killall testpocsag.sh >/dev/null 2>/dev/null ;;
			11\ *) sudo killall testopera.sh >/dev/null 2>/dev/null ;;
			12\ *) sudo killall testrtty.sh >/dev/null 2>/dev/null ;;
			
	esac		
}

do_status()
{
	LAST_ITEM="$menuchoice"
	whiptail --title "Transmit ""$LAST_ITEM"" on ""$OUTPUT_FREQ"" MHz" --msgbox "Transmitting" 8 78
	do_stop_transmit
}

#********************************
# User interface initialization *
#********************************

do_freq_setup

 while [ true ]
    do

	menuchoice=$(whiptail --default-item "$LAST_ITEM" --title "Rpitx on ""$OUTPUT_FREQ"" MHz" --menu "Range frequency : 50kHz-1GHz. Choose your test:" 20 82 12 \
 	"F Set frequency" "Modify frequency (actual $OUTPUT_FREQ MHz)" \
	"0 Tune" "Carrier" \
    "1 Chirp" "Moving carrier" \
	"2 Spectrum" "Spectrum painting" \
	"3 RfMyFace" "Snap with Raspicam and RF paint" \
	"4 FmRds" "Broadcast modulation with RDS" \
	"5 NFM" "Narrow band FM" \
	"6 SSB" "Upper Side Band modulation" \
	"7 AM" "Amplitude Modulation (Poor quality)" \
	"8 FreeDV" "Digital voice mode 800XA" \
	"9 SSTV" "Pattern picture" \
	"10 Pocsag" "Pager message" \
    "11 Opera" "Like morse but need Opera decoder" \
    "12 RTTY" "Radioteletype" \
 	3>&2 2>&1 1>&3)
		RET=$?
		if [ $RET -eq 1 ]; then
			whiptail --title "Bye bye" --msgbox "Thx for using rpitx" 8 78
    		exit 0
		elif [ $RET -eq 0 ]; then
			case "$menuchoice" in
			
			F\ *) do_freq_setup 
			;;
			
			0\ *) "./testvfo.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status
			;;
			
			1\ *) "./testchirp.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status
			;;
			
			2\ *) do_file_choose "320x256 .jpg" "$DEFAUL_JPG_PICTURE_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testspectrum.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			3\ *) "./snap2spectrum.sh" "$OUTPUT_FREQ""e6" >/dev/null 2>/dev/null &
			do_status
			;;
			
			4\ *) do_file_choose ".wav" "$DEFAULT_WAV_FILE_STEREO_LOC"
			if [ $abort_action -eq 0 ]; then
     			"./testfmrds.sh" "$OUTPUT_FREQ" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			5\ *) do_file_choose ".wav (16 bit per sample, 48000 sample rate, mono)" "$DEFAULT_WAV_FILE_MONO_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testnfm.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			6\ *) do_file_choose ".wav (16 bit per sample, 48000 sample rate, mono)" "$DEFAULT_WAV_FILE_MONO_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testssb.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			7\ *) do_file_choose ".wav (16 bit per sample, 48000 sample rate, mono)" "$DEFAULT_WAV_FILE_MONO_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testam.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			8\ *) do_file_choose "FreeDV .rf" "$DEFAULT_RF_FREEDV_FILE_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testfreedv.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			9\ *) do_file_choose "320x256 .jpg" "$DEFAUL_JPG_PICTURE_LOC"
			if [ $abort_action -eq 0 ]; then
				"./testsstv.sh" "$OUTPUT_FREQ""e6" "$FILE_LOC" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			10\ *) do_enter_message "POCSAG (ADDR:MESSAGE_BODY)" "$DEFAULT_POCSAG_MESSAGE"
			if [ $abort_action -eq 0 ]; then
				"./testpocsag.sh" "$OUTPUT_FREQ""e6" "$MESSAGE" >/dev/null 2>/dev/null &
				do_status
			fi
			;;

			11\ *) do_enter_callsign
			if [ $abort_action -eq 0 ]; then
				"./testopera.sh" "$OUTPUT_FREQ""e6" "$CALLSIGN" >/dev/null 2>/dev/null &
				do_status
			fi
			;;

			12\ *) do_enter_message "RTTY" "$DEFAULT_RTTY_MESSAGE"
			if [ $abort_action -eq 0 ]; then
				"./testrtty.sh" "$OUTPUT_FREQ""e6" "$MESSAGE" >/dev/null 2>/dev/null &
				do_status
			fi
			;;
			
			esac
		else
			exit 1
		fi
    done
	exit 0