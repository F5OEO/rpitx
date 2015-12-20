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
from ctypes import *
class Sample(Structure):
    _fields_ = [("amplitude", c_double), ("timing", c_uint)]

#with open("/tmp/upstream.bin", "rb") as file:
#    result = []
#    x = Pippo()
#    while file.readinto(x) == sizeof(x):
#        result.append((x.a, x.t))

BASE = [1, 2, 4, 8, 10, 20, 40, 80]
NUMS = 5
UP = 32767.0
DOWN = UP/8

filename = "/tmp/pippo.rfa"


# from https://git.s7t.de/dcf77/dcf77-python/blob/master/dcf77/dcfvalue.py
class DCF77Value:
  def __init__(self):
    self.hour = 00
    self.minute = 00
    self.second = 00
    self.day = 00
    self.month = 00
    self.year = 0000
    self.data = []
    self.parity = True
    self.data = []
    self.chunklist = {
        "startbit"              : [0, 1],
        "weather"               : [1, 14],
        "callbit"               : [15, 1],
        "cest_announce"         : [16, 1],
        "cest"                  : [17, 1],
        "cet"                   : [18, 1],
        "leap_second_announce"  : [19, 1],
        "start_time"            : [20, 1],
        "minute"                : [21, 7],
        "minute_parity"         : [28, 1],
        "hour"                  : [29, 6],
        "hour_parity"           : [35, 1],
        "day_month"             : [36, 6],
        "day_week"              : [42, 3],
        "month"                 : [45, 5],
        "year"                  : [50, 8],
        "date_parity"           : [58, 1],
        "date"                  : [36, 22],
        "minute_mark"           : [59, 1],
    }

  def getByte(self, byte):
    if len(self.data) >= byte:
      return self.data[byte]
    return False


  def getChunk(self, chunk):
    start = self.chunklist[chunk][0];
    end = start + self.chunklist[chunk][1];
    if len(self.data) >= end:
      return self.data[start:end]
    return False


  def getCurrentBitCount(self):
    return len(self.data)

  def setMinute(self):
    data = self.getChunk("minute")
    if data == False or len(data) < 7:
      return

    self.minute = (data[0] + (2 * data[1]) + (4 * data[2]) + (8 * data[3]) + (10 * data[4]) + (20 * data[5]) + (40 * data[6]))

  def setHour(self):
    data = self.getChunk("hour");
    print data
    if data == False or len(data) < 6:
      return

    self.hour = (data[0] + (2 * data[1]) + (4 * data[2]) + (8 * data[3]) + (10 * data[4]) + (20 * data[5]))

  def setDay(self):
    data = self.getChunk("day_month")
    if data == False or len(data) < 6:
      return

    self.day = (data[0] + (2 * data[1])  + (4 * data[2]) + (8 * data[3])  + (10 * data[4]) + (20 * data[5]))

  def setMonth(self):
    data = self.getChunk("month")
    if data == False or len(data) < 5:
      return

    self.month = (data[0] + (2 * data[1]) + (4 * data[2]) + (8 * data[3]) + (10 * data[4]))
  
  def setYear(self):
    data = self.getChunk("year")
    if data == False or len(data) < 8:
      return;

    self.year = (data[0] + (2 * data[1]) + (4 * data[2]) + (8 * data[3]) + (10 * data[4]) + (20 * data[5]) + (40 * data[6]) + (80 * data[7]) + 2000)
  
  def getMinute(self):
    return self.minute
  def getHour(self):
    return self.hour
  def getDay(self):
    return self.day
  def getMonth(self):
    return self.month
  def getYear(self):
    return self.year
  def getTime(self):
    return "%02d:%02d" % (self.getHour(), self.getMinute())
  def getDate(self):
    return "%02d.%02d.%04d" % (self.getDay(), self.getMonth(), self.getYear())

  def parse(self):
    self.setMinute()
    self.setHour()
    self.setDay()
    self.setMonth()
    self.setYear()
# fine 


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
    date.append(int_to_bcd(data.year - 2000, 8))
    time_code.append(add_crc("".join(date)))
    return "".join(time_code)

def modulate(value=None):
    if value is None:
        # last secons
        return [(UP,1000000000)]
    else:
        if value == 0:
            return [(DOWN,100000000),
                    (UP,900000000)]
        else:
            return [(DOWN,200000000),
                    (UP,800000000)]

def test(value = "000000000000000010010111000011011111000000011000010101000010"):
    sample = Sample()
    print value
    with open(filename, "wb") as f:
        for val in ( int(v) for v in value):
            for amplitude, timing in modulate(val):
                sample.amplitude = amplitude
                sample.timing = timing
                f.write(sample)
                print "%s  %i" % (amplitude, timing)



if __name__ == "__main__":    
    with open(filename, "wb") as f:
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
        sample = Sample()
        for i in range(NUMS):
            for val in ( int(v) for v in to_dcf77(start + datetime.timedelta(i))):
                print val, ":",
                for amplitude, timing in modulate(val):
                    sample.amplitude = amplitude
                    sample.timing = timing			
                    #f.write(struct.pack("!fI", amplitude, timing))
                    f.write(sample)
                    print "%s, %s" % (amplitude, timing)
            # inizio minuto
            print "chiusura :",
            for amplitude, timing in modulate():
                sample.amplitude = amplitude
                sample.timing = timing			
                #f.write(struct.pack("!fI", amplitude, timing))
                f.write(sample)
                print "%s, %s" % (amplitude, timing)
    trigger = start - datetime.timedelta(minutes=1)

    while True:
        if datetime.datetime.now() >= trigger:
            break
    call(["sudo", "rpitx", "-m", "RFA", "-i", filename, "-f", "77.500", "-l"])
