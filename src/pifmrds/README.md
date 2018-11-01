Pi-FM-RDS
=========


## FM-RDS transmitter using the Raspberry Pi

This program generates an FM modulation, with RDS (Radio Data System) data generated in real time. It can include monophonic or stereophonic audio.

This version modulates the PLL instead of the clock divider for superior signal purity. The harmonics are unaffected, so the [legal warning](#warning-and-disclaimer) still applies.

![](doc/spectrum.png)

TODO list
*watchdog for PLL settings to prevent radio interference
*measure PLL loop filter response

It is based on the FM transmitter created by [Oliver Mattos and Oskar Weigl](http://www.icrobotics.co.uk/wiki/index.php/Turning_the_Raspberry_Pi_Into_an_FM_Transmitter), and later adapted to using DMA by [Richard Hirst](https://github.com/richardghirst). Christophe Jacquet adapted it and added the RDS data generator and modulator. The transmitter uses the Raspberry Pi's clock divider to produce VHF signals.

It is compatible with both the Raspberry Pi 1 (the original one) and the Raspberry Pi 2 and 3.  Users of Raspberry Pi 3 should add gpu_freq=250 to /boot/config.txt .  The Pi 3 has very sensitive low voltage detection. When low voltage is detected, clocks are reduced to safe values in an attempt to prevent crashes. This program changes clocks to generate the desired radio frequency without the knowledge of the power management system. While it would be possible to detect and undo changes, this would cause radio interference each time it happens. Setting gpu_freq=250 appears to prevent undesired clock changes because the normal value and safe value are the same. 

![](doc/vfd_display.jpg)

PiFmRds has been developed for experimentation only. It is not a media center, it is not intended to broadcast music to your stereo system. See the [legal warning](#warning-and-disclaimer).

## How to use it?

Pi-FM-RDS, depends on the `sndfile` library. To install this library on Debian-like distributions, for instance Raspbian, run `sudo apt-get install libsndfile1-dev`.

Pi-FM-RDS also depends on the Linux `rpi-mailbox` driver, so you need a recent Linux kernel. The Raspbian releases from August 2015 have this.

**Important.** The binaries compiled for the Raspberry Pi 1 are not compatible with the Raspberry Pi 2/3, and conversely. Always re-compile when switching models, so do not skip the `make clean` step in the instructions below!

Clone the source repository and run `make` in the `src` directory:

```bash
git clone https://github.com/F5OEO/PiFmRds.git
cd PiFmRds/src
git clone https://github.com/F5OEO/librpitx.git
cd librpitx/src
make
cd ../../
make clean
make
```

Then you can just run:

```
sudo ./pi_fm_rds
```

This will generate an FM transmission on 107.9 MHz, with default station name (PS), radiotext (RT) and PI-code, without audio. The radiofrequency signal is emitted on GPIO 4 (pin 7 on header P1).


You can add monophonic or stereophonic audio by referencing an audio file as follows:

```
sudo ./pi_fm_rds -audio sound.wav
```

To test stereophonic audio, you can try the file `stereo_44100.wav` provided.

The more general syntax for running Pi-FM-RDS is as follows:

```
pi_fm_rds [-freq freq] [-audio file] [-ppm ppm_error] [-pi pi_code] [-ps ps_text] [-rt rt_text]
```

All arguments are optional:

* `-freq` specifies the carrier frequency (in MHz). Example: `-freq 107.9`.
* `-audio` specifies an audio file to play as audio. The sample rate does not matter: Pi-FM-RDS will resample and filter it. If a stereo file is provided, Pi-FM-RDS will produce an FM-Stereo signal. Example: `-audio sound.wav`. The supported formats depend on `libsndfile`. This includes WAV and Ogg/Vorbis (among others) but not MP3. Specify `-` as the file name to read audio data on standard input (useful for piping audio into Pi-FM-RDS, see below).
* `-pi` specifies the PI-code of the RDS broadcast. 4 hexadecimal digits. Example: `-pi FFFF`.
* `-ps` specifies the station name (Program Service name, PS) of the RDS broadcast. Limit: 8 characters. Example: `-ps RASP-PI`.
* `-rt` specifies the radiotext (RT) to be transmitted. Limit: 64 characters. Example: `-rt 'Hello, world!'`.
* `-ctl` specifies a named pipe (FIFO) to use as a control channel to change PS and RT at run-time (see below).
* `-ppm` specifies your Raspberry Pi's oscillator error in parts per million (ppm), see below.

By default the PS changes back and forth between `Pi-FmRds` and a sequence number, starting at `00000000`. The PS changes around one time per second.


### Clock calibration (only if experiencing difficulties)

The RDS standards states that the error for the 57 kHz subcarrier must be less than ± 6 Hz, i.e. less than 105 ppm (parts per million). The Raspberry Pi's oscillator error may be above this figure. That is where the `-ppm` parameter comes into play: you specify your Pi's error and Pi-FM-RDS adjusts the clock dividers accordingly.

In practice, I found that Pi-FM-RDS works okay even without using the `-ppm` parameter. I suppose the receivers are more tolerant than stated in the RDS spec.

One way to measure the ppm error is to play the `pulses.wav` file: it will play a pulse for precisely 1 second, then play a 1-second silence, and so on. Record the audio output from a radio with a good audio card. Say you sample at 44.1 kHz. Measure 10 intervals. Using [Audacity](http://audacity.sourceforge.net/) for example determine the number of samples of these 10 intervals: in the absence of clock error, it should be 441,000 samples. With my Pi, I found 441,132 samples. Therefore, my ppm error is (441132-441000)/441000 * 1e6 = 299 ppm, **assuming that my sampling device (audio card) has no clock error...**


### Piping audio into Pi-FM-RDS

If you use the argument `-audio -`, Pi-FM-RDS reads audio data on standard input. This allows you to pipe the output of a program into Pi-FM-RDS. For instance, this can be used to read MP3 files using Sox:

```
sox -t mp3 http://www.linuxvoice.com/episodes/lv_s02e01.mp3 -t wav -  | sudo ./pi_fm_rds -audio -
```

Or to pipe the AUX input of a sound card into Pi-FM-RDS:

```
sudo arecord -fS16_LE -r 44100 -Dplughw:1,0 -c 2 -  | sudo ./pi_fm_rds -audio -
```


### Changing PS, RT and TA at run-time

You can control PS, RT and TA (Traffic Announcement flag) at run-time using a named pipe (FIFO). For this run Pi-FM-RDS with the `-ctl` argument.

Example:

```
mkfifo rds_ctl
sudo ./pi_fm_rds -ctl rds_ctl
```

Then you can send “commands” to change PS, RT and TA:

```
cat >rds_ctl
PS MyText
RT A text to be sent as radiotext
TA ON
PS OtherTxt
TA OFF
...
```

Every line must start with either `PS`, `RT` or `TA`, followed by one space character, and the desired value. Any other line format is silently ignored. `TA ON` switches the Traffic Announcement flag to *on*, any other value switches it to *off*.


## Warning and Disclaimer

PiFmRds is an **experimental** program, designed **only for experimentation**. It is in no way intended to become a personal *media center* or a tool to operate a *radio station*, or even broadcast sound to one's own stereo system.

In most countries, transmitting radio waves without a state-issued licence specific to the transmission modalities (frequency, power, bandwidth, etc.) is **illegal**.

Therefore, always connect a shielded transmission line from the RaspberryPi directly
to a radio receiver, so as **not** to emit radio waves. Never use an antenna.

Even if you are a licensed amateur radio operator, using PiFmRds to transmit radio waves on ham frequencies without any filtering between the RaspberryPi and an antenna is most probably illegal because the square-wave carrier is very rich in harmonics, so the bandwidth requirements are likely not met.

I could not be held liable for any misuse of your own Raspberry Pi. Any experiment is made under your own responsibility.


## Tests

Pi-FM-RDS was successfully tested with all my RDS-able devices, namely:

* a Sony ICF-C20RDS alarm clock from 1995,
* a Sangean PR-D1 portable receiver from 1998, and an ATS-305 from 1999,
* a Samsung Galaxy S2 mobile phone from 2011,
* a Philips MBD7020 hifi system from 2012,
* a Silicon Labs [USBFMRADIO-RD](http://www.silabs.com/products/mcu/Pages/USBFMRadioRD.aspx) USB stick, employing an Si4701 chip, and using my [RDS Surveyor](http://rds-surveyor.sourceforge.net/) program,
* a “PCear Fm Radio”, a Chinese clone of the above, again using RDS Surveyor.

Reception works perfectly with all the devices above. RDS Surveyor reports no group errors.

![](doc/galaxy_s2.jpg)


### CPU Usage

CPU usage on a Raspberry Pi 1 is as follows:

* without audio: 9%
* with mono audio: 33%
* with stereo audio: 40%

CPU usage increases dramatically when adding audio because the program has to upsample the (unspecified) sample rate of the input audio file to 228 kHz, its internal operating sample rate. Doing so, it has to apply an FIR filter, which is costly.

## Design

The RDS data generator lies in the `rds.c` file.

The RDS data generator generates cyclically four 0A groups (for transmitting PS), and one 2A group (for transmitting RT). In addition, every minute, it inserts a 4A group (for transmitting CT, clock time). `get_rds_group` generates one group, and uses `crc` for computing the CRC.

To get samples of RDS data, call `get_rds_samples`. It calls `get_rds_group`, differentially encodes the signal and generates a shaped biphase symbol. Successive biphase symbols overlap: the samples are added so that the result is equivalent to applying the shaping filter (a [root-raised-cosine (RRC) filter ](http://en.wikipedia.org/wiki/Root-raised-cosine_filter) specified in the RDS standard) to a sequence of Manchester-encoded pulses.

The shaped biphase symbol is generated once and for all by a Python program called `generate_waveforms.py` that uses [Pydemod](https://github.com/ChristopheJacquet/Pydemod), one of my other software radio projects. This Python program generates an array called `waveform_biphase` that results from the application of the RRC filter to a positive-negative impulse pair. *Note that the output of `generate_waveforms.py`, two files named `waveforms.c` and `waveforms.h`, are included in the Git repository, so you don't need to run the Python script yourself to compile Pi-FM-RDS.*

Internally, the program samples all signals at 228 kHz, four times the RDS subcarrier's 57 kHz.

The FM multiplex signal (baseband signal) is generated by `fm_mpx.c`. This file handles the upsampling of the input audio file to 228 kHz, and the generation of the multiplex: unmodulated left+right signal (limited to 15 kHz), possibly the stereo pilot at 19 kHz, possibly the left-right signal, amplitude-modulated on 38 kHz (suppressed carrier) and RDS signal from `rds.c`. Upsampling is performed using a polyphase filter bank of 32 filters, each with 32 coefficients. To help understand the polyphase filter bank, consider upsampling the input signal by zero stuffing. Then, apply a low pass filter with the cutoff at the original Nyquist frequency.  Finally, collect some of the filtered samples at the new sampling rate. The polyphase filter bank does the same thing mathematically, but avoids computing output samples that will not be used. It also avoids processing all of the suffed zeros. The low pass part of the filter is a sampled sinc. The filter is also used to provide pre-emphasis. The low pass filter coefficients are convolved with the pre-emphasis filter, providing pre-emphasis at no additional cost. The combined filter is windowed by a Hamming window. The filter coefficients are generated at startup so that the filter cuts frequencies above the minimum of:
* the Nyquist frequency of the input audio file (half the sample rate) to avoid aliasing (this is the typical case for resampling),
* 15 kHz, the bandpass of the left+right and left-right channels, as per the FM broadcasting standards.

An Octave script to compute the frequency response of the filter is provided in the doc folder.

The samples are played by `pi_fm_rds.c` that is adapted from Richard Hirst's [PiFmDma](https://github.com/richardghirst/PiBits/tree/master/PiFmDma). The program was changed to support a sample rate of precisely 228 kHz.


### References

* [EN 50067, Specification of the radio data system (RDS) for VHF/FM sound broadcasting in the frequency range 87.5 to 108.0 MHz](http://www.interactive-radio-system.com/docs/EN50067_RDS_Standard.pdf)


## History
* 2018-11-01: Integrate in rpitx project. Hope not to hurt the author , copying readme and licence. 
* 2018-03-19: Use librpitx for easy integration
* 2015-09-05: support for the Raspberry Pi 2
* 2014-11-01: support for toggling the Traffic Announcement (TA) flag at run-time
* 2014-10-19: bugfix (cleanly stop the DMA engine when the specified file does not exist, or it's not possible to read from stdin)
* 2014-08-04: bugfix (ppm now uses floats)
* 2014-06-22: generate CT (clock time) signals, bugfixes
* 2014-05-04: possibility to change PS and RT at run-time
* 2014-04-28: support piping audio file data to Pi-FM-RDS' standard input
* 2014-04-14: new release that supports any sample rate for the audio input, and that can generate a proper FM-Stereo signal if a stereophonic input file is provided
* 2014-04-06: initial release, which only supported 228 kHz monophonic audio input files

--------

© [Christophe Jacquet](http://www.jacquet80.eu/) (F8FTK), 2014-2015. Released under the GNU GPL v3.
