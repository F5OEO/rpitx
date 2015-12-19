#!/usr/local/bin/python2.7
# encoding: utf-8

"""
inspired by https://github.com/CodingGhost/DCF77-Transmitter

reference
https://en.wikipedia.org/wiki/DCF77

"""

import datetime

import struct
from subprocess import call

BASE = [1, 2, 4, 8, 10, 20, 40, 80]
NUMS = 5
UP = 32767
DOWN = 3276
filename = "/tmp/pippo.rfa"

def int_to_bcd(value, num_bits):
    """
    
    """
    bits = []
    for base in reversed(BASE[:num_bits]):
        if value >= base:
            bits.append("1")
            value -= base
        else:
            bits.append("0")
    return "".join(reversed(bits))

def add_crc(value, odd=True):
    num = value.count("1")
    p = num % 2
    if(( p == 1 ) and odd) or ( p==0 and not odd) :
        return "%s1" % value
    else:
        return "%s0" % value

def to_dcf77(data):
    time_code = ["011111111100000",]
    time_code.append("11")
    if data.dst():
        time_code.append("10")
    else:
        time_code.append("01")
    time_code.append("0") #leap second
    time_code.append("1")
    time_code.append(add_crc(int_to_bcd(data.minute, 7)))
    time_code.append(add_crc(int_to_bcd(data.hour, 6)))
    date = [int_to_bcd(data.day, 6),]
    date.append(int_to_bcd(data.weekday()+1, 3))
    date.append(int_to_bcd(data.month, 5))
    date.append(int_to_bcd(data.year, 8))
    time_code.append(add_crc("".join(date)))
    return "".join(time_code)

def modulate(value=None):
    if value:
        if value == 0:
            return [(DOWN,100000000),
                    (UP,900000000)]
        else:
            return [(DOWN,200000000),
                    (UP,800000000)]
    else:
        # last secons
        return [(UP,1000000000)]
    
with open(filename, "w") as f:
    now = datetime.datetime.now()
    # tolgo secondi per arrivare a inizio minuto
    # aggiungo 1 per arrivare a prossimo minuto
    # aggiungo 1 perchè ad ogni minuto, comunico il minuto dopo
    start = now  + datetime.timedelta(minutes=2) - \
            datetime.timedelta(microseconds = now.microsecond,
                               seconds = now.second )
    if now.second >= 58:
        # not enougth time
        start = start + datetime.timedelta(minutes=1)

    for i in range(NUMS):
        for amplitude, timing in modulate(to_dcf77(start + datetime.timedelta(i))):
            f.write(struct.pack("dI", amplitude, timing))
trigger = start - datetime.timedelta(minutes=1)

while True:
    if datetime.datetime.now() >= trigger:
        break
call(["sudo", "rpitx", "-m", "RFA", "-i", filename, "-f", "77.500", "-l"])
