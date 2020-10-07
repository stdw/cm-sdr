# cm-sdr

cm-sdr is a piece of software to turn a Broadcom BCM3383-based cable modem into
a general purpose software defined radio.

## Status

In its current state, cm-sdr serves as a proof of concept. It can currently
stream I/Q data to a peer via TCP. A script to demodulate and play FM radio
broadcasts is also provided.

cm-sdr is dependent on many functions and memory mapped registers in the
unmodified firmare. Thus, at the moment it is highly tailored to one specific
cable modem model and firmware version.

## TODO

* Figure out how to decrease sample rate
* Transmitting
* Usability improvemnts
* Porting

## Usage

### Building

* Download the toolchain: https://github.com/Broadcom/stbgcc-4.8/releases/download/stbgcc-4.8-1.0/stbgcc-4.8-1.0.tar.bz2
* Extract it and add it to your path `export PATH=/opt/toolchains/stbgcc-4.8-1.0/bin/:$PATH`
* Build with `make`

### Running
* Ensure Python 3 is installed
* Load binary with the provided python script `python3 upload.py`
* Connect to modem and begin recieving data `nc 192.168.100.1 1337 > data`
* To run FM demo, ensure numpy, scipy, and sounddevice are installed
* Use the tune.py script and netcat to set the frequency and pipe the data into the demodulation script `python3 tune.py --freq 107900000 | nc 192.168.100.1 1337 | python3 fm-stream.py`
