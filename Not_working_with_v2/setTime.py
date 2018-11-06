#!/usr/bin/env python
# encoding: utf-8

"""
inspired by https://github.com/CodingGhost/DCF77-Transmitter

reference
https://en.wikipedia.org/wiki/DCF77

"""
import argparse
from ctypes import *
import datetime
import logging
import os
import struct
from subprocess import call
import tempfile

logging.basicConfig()
logger = logging.getLogger(__name__)


class Sample(Structure):
    '''
    Helper for writing binary data to RFA file
    '''
    _fields_ = [("amplitude", c_double), ("timing", c_uint)]

    def __init__(self, amplitude, timing):
        self.amplitude = amplitude
        self.timing = timing


class Dcf77(object):
    '''
    Utility for working dcf77 standard
    https://en.wikipedia.org/wiki/DCF77
    '''

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
        see https://en.wikipedia.org/wiki/DCF77

        :param data: datetime.datetime 
        '''
        time_code = ["0",  # start of minute
                     "11111111100000",  # wheather, warnngs or other data
                     "1",  # Abnormal transmitter / backup antenna
                     "0",  # Summer time announcement
                     ]

        time_code.append("00")  # cest and cet bit
        time_code.append("0")  # leap second
        time_code.append("1")  # start encoding time

        time_code.append(cls.add_crc(cls.int_to_bcd(data.minute, 7)))
        time_code.append(cls.add_crc(cls.int_to_bcd(data.hour, 6)))
        date = [cls.int_to_bcd(data.day, 6), ]
        date.append(cls.int_to_bcd(data.weekday() + 1, 3))
        date.append(cls.int_to_bcd(data.month, 5))
        date.append(cls.int_to_bcd(data.year - 2000, 8))
        time_code.append(cls.add_crc("".join(date)))

        return "".join(time_code)

    @staticmethod
    def modulate(encoded, UP=32767, DOWN=0):
        '''
        dcf77 modulation
        1 bit per second
        0.1 sec down = 0
        0.2 sec down = 1
        1 sec up = end of second

        return generator of tuple returning (value, time in milliseconds)

        :param encoded: sequence of 01 representing message
        :param UP:  value for 100% modulation
        :param DOWN: value for 15% modulation
                     I do not know why, but 0 seems to work
        '''

        for value in encoded:

            if value in ("0", 0):
                yield (DOWN, 100000000)
                yield (UP, 900000000)
            elif value in ("1", 1):
                yield (DOWN, 200000000)
                yield (UP, 800000000)
            else:
                raise ValueError("Only 0|1")
        # last second no modulation
        yield (UP, 1000000000)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Call rpitx to set dcf77 clock",
        epilog="default wire on GPIO 18 ( pin 12) and 5 minutes of transmission")
    parser.add_argument("-v", "--verbose", help="increase output verbosity",
                        action="store_true")
    parser.add_argument("-4", "--gpio4", help="wire on GPIO 4 ( pin 7)",
                        action="store_true")
    parser.add_argument("-n", "--nums", help="number of minutes  of transmission",
                        type=int,
                        default=5)
    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    NUMS = args.nums  # transmit for NUMS minutes
    now = datetime.datetime.now()
    # tolgo secondi per arrivare a inizio minuto
    # aggiungo 1 per arrivare a prossimo minuto
    # aggiungo 1 perchè ad ogni minuto, comunico il minuto dopo
    start = now - datetime.timedelta(microseconds=now.microsecond,  # start of current minute
                                     seconds=now.second) + \
        datetime.timedelta(minutes=1)  # start of next minute

    if now.second >= 58:
        # not enougth time, add one more minute
        start = start + datetime.timedelta(minutes=1)

    filename = None
    with tempfile.NamedTemporaryFile("wb", delete=False) as f:
        filename = f.name

        for i in range(1, NUMS + 1):
            # i start with 1 because now whi transmit next minute ( now+1
            # minute)
            data = start + datetime.timedelta(minutes=i)
            encoded = Dcf77.to_dcf77(data)

            logger.debug("%s -> %s", data, encoded)

            for amplitude, timing in Dcf77.modulate(encoded):
                sample = Sample(amplitude, timing)
                f.write(sample)
                logger.debug("%s, %s", amplitude, timing)

    print "waiting %s for syncronization..." % start
    while True:
        # whaiting for start of minute
        if datetime.datetime.now() >= start:
            break
    cmd = ["sudo", "./rpitx", "-m", "RFA", "-i",
           filename, "-f", "77.500"]
    if args.gpio4:
        cmd.extend(["-c", "1"])
    logger.debug(cmd)

    ret = call(cmd)
    logger.debug("ret = %s", ret)
    if ret == 0:
        print "End of transmission, hope your clock is set."
    else:
        print "Something got wrong, check for errors"
    if filename is not None:
        os.remove(filename)
