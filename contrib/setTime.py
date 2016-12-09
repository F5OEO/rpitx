#!/usr/local/bin/python2.7
# encoding: utf-8

"""
inspired by https://github.com/CodingGhost/DCF77-Transmitter

reference
https://en.wikipedia.org/wiki/DCF77

"""

from ctypes import *
import datetime
import struct
from subprocess import call
import tempfile


class Sample(Structure):
    _fields_ = [("amplitude", c_double), ("timing", c_uint)]

    def __init__(self, amplitude, timing):
        self.amplitude = amplitude
        self.timing = timing

#     @staticmethod
#     def read(filename):
#         with open(filename, "rb") as file:
#             result = []
#             x = Sample()
#             while file.readinto(x) == sizeof(x):
#                 result.append((x.amplitude, x.timing))
#             return result


class Dcf77(object):

    BASE = [1, 2, 4, 8, 10, 20, 40, 80]

    @staticmethod
    def int_to_bcd(value, num_bits):
        """

        """
        bits = []
        for base in reversed(Dcf77.BASE[:num_bits]):
            if value >= base:
                bits.append("1")
                value -= base
            else:
                bits.append("0")
        return "".join(reversed(bits))

    @staticmethod
    def add_crc(value, odd=True):
        num = value.count("1")
        p = num % 2
        if((p == 1) and odd) or (p == 0 and not odd):
            return "%s1" % value
        else:
            return "%s0" % value

    @classmethod
    def to_dcf77(cls, data):
        '''
        Convert datetime to list of bit in dcf77 encoding

        :param data: datetime.datetime 
        '''
        time_code = ["011111111100000", ]
        time_code.append("11")
        if data.dst():
            time_code.append("10")
        else:
            time_code.append("01")
        time_code.append("0")  # leap second
        time_code.append("1")
        time_code.append(cls.add_crc(cls.int_to_bcd(data.minute, 7)))
        time_code.append(cls.add_crc(cls.int_to_bcd(data.hour, 6)))
        date = [cls.int_to_bcd(data.day, 6), ]
        date.append(cls.int_to_bcd(data.weekday() + 1, 3))
        date.append(cls.int_to_bcd(data.month, 5))
        date.append(cls.int_to_bcd(data.year - 2000, 8))
        time_code.append(cls.add_crc("".join(date)))
        return "".join(time_code)

    @staticmethod
    def modulate(encoded, UP=32767.0, ratio=0.15):
        '''




        :param value:
        :param UP:  value for 100% modulation
        :param ratio: low modulation ( standard is 15% )
        '''

        DOWN = 0

        for value in encoded:

            if value in ("0", 0):
                yield (DOWN, 100000000)
                yield (UP, 900000000)
            elif value in ("1", 1):
                yield (DOWN, 200000000)
                yield (UP, 800000000)
            else:
                raise ValueError("Only None|0|1")
        # last second no modulation
        yield (UP, 1000000000)


def test(value="000000000000000010010111000011011111000000011000010101000010"):
    sample = Sample()
    print value
    with open(filename, "wb") as f:
        for val in (int(v) for v in value):
            for amplitude, timing in modulate(val):
                sample.amplitude = amplitude
                sample.timing = timing
                f.write(sample)
                print "%s  %i" % (amplitude, timing)


if __name__ == "__main__":

    NUMS = 5  # transmit for NUMS minutes
    now = datetime.datetime.now()
    # tolgo secondi per arrivare a inizio minuto
    # aggiungo 1 per arrivare a prossimo minuto
    # aggiungo 1 perchè ad ogni minuto, comunico il minuto dopo
    start = now  + datetime.timedelta(minutes=2) - \
        datetime.timedelta(microseconds=now.microsecond,
                           seconds=now.second)
    if now.second >= 58:
        # not enougth time
        start = start + datetime.timedelta(minutes=1)

    filename = None
    with tempfile.NamedTemporaryFile("wb", delete=False) as f:
        filename = f.name

        for i in range(NUMS):
            data = start + datetime.timedelta(seconds=60 * i)
            encoded = Dcf77.to_dcf77(data)

            print "%s -> %s" % (data, encoded)

            for amplitude, timing in Dcf77.modulate(encoded):
                sample = Sample(amplitude, timing)
                f.write(sample)
                print "%s, %s" % (amplitude, timing)

    trigger = start - datetime.timedelta(minutes=1)

    while True:
        # whaiting for start of minute
        if datetime.datetime.now() >= trigger:
            break
    cmd = ["sudo", "rpitx", "-m", "RFA", "-i",
           filename, "-f", "77.500", "-c", "1"]
    print cmd
    call(cmd)
    print "FINITO !!!"
