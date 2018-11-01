![rpitx banner](/doc/Galois-pastemps.jpg)
# About rpitx
**rpitx** is a general radio frequency transmitter for Raspberry Pi which doesn't require any other hardware unless filter to avoid intererence. It can handle frequencies from 5 KHz up to 1500 MHz.

Rpitx is a software made for educational on RF system. It has not been tested for compliance with regulations governing transmission of radio signals. You are responsible for using your Raspberry Pi legally.

_Created by Evariste Courjaud F5OEO. See Licence for using it.

# Installation

Assuming a Raspbian Lite installation (stretch) : https://www.raspberrypi.org/downloads/raspbian/

Be sure to have git package installed :
```sh
sudo apt-get update
sudo apt-get install git
```
You can now clone the repository. A script (install.sh) is there for easy installation. You could inspect it and make steps manualy in case of any doubt. You can note that /boot/config.txt should be prompt to be modified during the installation. If it is not accepted, **rpitx** will be unstable.  

```sh
git clone https://github.com/F5OEO/rpitx
cd rpitx
git checkout v2beta
./install.sh
```
Make a reboot in order to use **rpitx** in a stable state.
That's it !
```sh
sudo reboot
```

# Hardware
![bpf](/doc/bpf-warning.png)

Plug a wire on GPIO 4, means Pin 7 of the GPIO header ([header P1](http://elinux.org/RPi_Low-level_peripherals#General_Purpose_Input.2FOutput_.28GPIO.29)). This acts as the antenna. The optimal length of the wire depends the frequency you want to transmit on, but it works with a few centimeters for local testing.

# How to use it
![easymenu](/doc/easymenu.png)
## Easytest
**easytest** is the easiest way to start and see some demonstration. All transmission are made on free ISM band (434MHZ).
To launch it, go to rpitx folder and launch easytest.sh :
```sh
cd rpitx
./easytest.sh
```
Easy way to monitor what you are doing is by using a SDR software and a SDR receiver like a rtl-sdr one and set the frequency to 434MHZ.

### Carrier ### 
![Carrier](/doc/Tunerpitx.png)
A simple carrier generated at 434MHZ. 

### Chirp ### 
![Chirp](/doc/chirprpitx.png)
A carrier which move around 434MHZ.

### Spectrum ###
![Spectrum](/doc/spectrumrpitx.png)
A picture is displayed on the waterfall on your SDR. Note that you should make some tweaks in order to obtain contrast and correct size depending on your reception and SDR software you use. 

### FM with RDS ###
![FMRDS](/doc/fmrds.png)
Broadcast FM with RDS. You should receive it with your SDR. This is the modulation that you should hear on your classical FM Radio receiver, but at this time, the frequency is too high.

### Single Side Band modulation (SSB) ###
![SSB](/doc/ssbrpitx.png)
This is the classical Hamradio analog voice modulation. Use your SDR in USB mode.

### Slow Scan Television (SSTV) ###
![SSTV](/doc/sstvrpitx.JPG)
This is a picture transmission mode using audio modulation (USB mode). You need an extra software to decode and display it (qsstv,msstv...). This demo uses the Martin1 mode of sstv.

### Opera (Beacon) ###
![opera](/doc/operarpitx.JPG)
This a beacon mode which sound like Morse. You need opera in mode 0.5 to decode.

### Pocsag (pager mode) ###
![pocsag](/doc/pocsagrpitx.JPG)
This is a mode used by pagers. You need an extra software to decode. Set your SDR in NBFM mode.

### Freedv (digital voice) ###
![freedv](/doc/freedvrpitx.JPG)
This is state of the art opensource digital modulation. You need Freedv for demodulation.

## To continue
**rpitx** is a generic RF transmitter. There is a lot of modulation to do with it and also documentation to make all that easy to contribute. This will be the next step ! Feel free to inspect scripts, change parameters (frequencies, audio input, pictures...). 

# Credits
All rights of the original authors reserved.
I try to include all licences and authors in sourcecode. Need to write all references in this section.  
