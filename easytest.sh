#!/bin/bash
status="0"
 while [ "$status" -eq 0 ] 
    do

 menuchoice=$(whiptail --title "Rpitx on 434Mhz" --menu "Choose your test, ctrl^c to end a test" 16 82 8 \
	"0 Tune" "Carrier" \
    "1 Chirp" "Moving carrier" \
	"2 Spectrum" "Spectrum painting" \
	"3 FmRds" "Broadcast modulation with RDS" \
	"4 SSB" "USB modulation" \
	"5 SSTV" "Patern picture" \
	"6 Opera" "Like morse but need Opera decoder" \
	"7 Pocsag" "Pager message" \
    "8 FreeDV" "Digital voice mode 800XA" \
 	3>&2 2>&1 1>&3)

        case "$menuchoice" in
	    0\ *) "./testvfo.sh" >/dev/null 2>/dev/null   ;;
        1\ *) "./testchirp.sh" >/dev/null 2>/dev/null   ;;
	    2\ *) "./testspectrum.sh" >/dev/null 2>/dev/null ;;
   	    3\ *) "./testfmrds.sh" >/dev/null 2>/dev/null ;;
	    4\ *) "./testssb.sh" >/dev/null 2>/dev/null ;;
	    5\ *) "./testsstv.sh" >/dev/null 2>/dev/null ;;
	    6\ *) "./testopera.sh" >/dev/null 2>/dev/null ;;
        7\ *) "./testpocsag.sh" >/dev/null 2>/dev/null ;;
		8\ *) "./testfreedv.sh" >/dev/null 2>/dev/null ;;
        *)	 status=1;;
        esac
       
    done
exit
