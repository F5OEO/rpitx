#!/bin/sh

status="0"
OUTPUT_FREQ=14.074
LAST_ITEM="0 CQ"
OUTPUT_CALL=""
OUTPUT_GRID="JN06"

OM_CALL=""
OM_LEVEL="10"
FREETEXT="RPITX FT8 PI"

OUTPUT_OFFSET="1240"
TIMESLOT="1"

do_offset_frequency()
{
    if OFFSET=$(whiptail --inputbox "Choose FT8 offset (10-2600Hz) Default is 1240Hz" 8 78 $OUTPUT_OFFSET --title "Offset Frequency" 3>&1 1>&2 2>&3); then
        OUTPUT_OFFSET=$OFFSET
    fi
}

do_slot_choice()
{
    if (whiptail --title "Time slot" --yesno --yes-button 0 --no-button 1 "Which timeslot (current) $TIMESLOT ?" 8 78 3>&1 1>&2 2>&3); then
        TIMESLOT="0"
    else 
        TIMESLOT="1"
    fi    
}

do_freq_setup()
{

    if FREQ=$(whiptail --inputbox "Choose FT8 output Frequency (in MHZ) Default is 14.074MHZ" 8 78 $OUTPUT_FREQ --title "Transmit Frequency" 3>&1 1>&2 2>&3); then
        OUTPUT_FREQ=$FREQ
    fi

    if CALL=$(whiptail --inputbox "Type your call" 8 78 $OUTPUT_CALL --title "Hamradio call" 3>&1 1>&2 2>&3); then
        OUTPUT_CALL=$CALL
    fi

    if GRID=$(whiptail --inputbox "Type your grid on 4 char:ex JN06" 8 78 $OUTPUT_GRID --title "Hamradio grid" 3>&1 1>&2 2>&3); then
        OUTPUT_GRID=$GRID
    fi
    do_offset_frequency
    do_slot_choice
}

do_status()
{
	LAST_ITEM="$menuchoice"
	whiptail --title "Processing ""$LAST_ITEM"" on ""$OUTPUT_FREQ""MHZ" --msgbox "Running" 8 78
	
}

do_om_call()
{
    

    if CALL=$(whiptail --inputbox "Input new OM" 8 78 $OM_CALL --title "OM Call" 3>&1 1>&2 2>&3); then
        OM_CALL=$CALL
    fi

    #init level could not be a "-", remove the init
    if LEVEL=$(whiptail --inputbox "Received level" 8 78 "0" --title "Received level" 3>&1 1>&2 2>&3); then
        OM_LEVEL=$LEVEL
    fi
    
}

do_freetext()
{
    

    if TEXT=$(whiptail --inputbox "Type free text(13 chars)" 8 78 $FREETEXT --title "FreeText" 3>&1 1>&2 2>&3); then
        FREETEXT=$TEXT
        
    fi

    if (whiptail --title "FreeText" --yesno "Transmit now ?" 8 78 3>&1 1>&2 2>&3); then
        sudo pift8 -f "$OUTPUT_FREQ"e6 "-m $FREETEXT"
    fi    
    
}

do_freq_setup

 while [ "$status" -eq 0 ]
    do

 menuchoice=$(whiptail --default-item "$LAST_ITEM" --title "FT8 with rpitx Slot $TIMESLOT Offset $OUTPUT_OFFSET" --menu "Choose your item" 20 82 12 \
	"0 CQ" "Calling CQ on $OUTPUT_FREQ" \
	"1 ENTER OM" "Input OM call" \
    "2 dB" "Answer Db" \
	"3 RRR" "Answer RRR "\
	"4 Grid" "Answer with grid" \
	"5 R+dB" "Answer with R+level" \
    "6 73" "Answer with 73" \
    "7 Text" "Free text" \
    "8 Refine" "Adjust offset/slot" \
	3>&2 2>&1 1>&3)

        case "$menuchoice" in
		0\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "CQ $OUTPUT_CALL $OUTPUT_GRID" -o "$OUTPUT_OFFSET" -s "$TIMESLOT" 
        LAST_ITEM="1 ENTER OM" ;;
        1\ *) do_om_call 
        LAST_ITEM="2 dB" ;;
		2\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "$OM_CALL $OUTPUT_CALL $OM_LEVEL" -o "$OUTPUT_OFFSET" -s "$TIMESLOT"
        LAST_ITEM="3 RRR" ;;
		3\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "$OM_CALL $OUTPUT_CALL RR73" -o "$OUTPUT_OFFSET" -s "$TIMESLOT"
        LAST_ITEM="0 CQ" ;;
		4\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "$OM_CALL $OUTPUT_CALL $OUTPUT_GRID" -o "$OUTPUT_OFFSET" -s "$TIMESLOT"
        LAST_ITEM="5 R+dB" ;;
		5\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "$OM_CALL $OUTPUT_CALL R$OM_LEVEL" -o "$OUTPUT_OFFSET" -s "$TIMESLOT"
        LAST_ITEM="6 73" ;;
		6\ *) sudo pift8 -f "$OUTPUT_FREQ"e6 -m "$OM_CALL $OUTPUT_CALL 73" -o "$OUTPUT_OFFSET" -s "$TIMESLOT"
        do_om_call
        LAST_ITEM="4 Grid" ;;
        7\ *) do_freetext ;;
        8\ *) do_offset_frequency
        do_slot_choice 
         LAST_ITEM="0 CQ" ;;
    	*)	 status=1
		whiptail --title "Bye bye" --msgbox "Thanks for using rpitx!" 8 78
		;;
        esac
    done
