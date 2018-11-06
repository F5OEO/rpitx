#!/usr/bin/python


#   PiFmRds - FM/RDS transmitter for the Raspberry Pi
#   Copyright (C) 2014 Christophe Jacquet, F8FTK
#   
#   See https://github.com/ChristopheJacquet/PiFmRds
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

#   This program generates a WAV file with a 1-second sine wave at 440 Hz,
#   followed by a 1-second silence.


import scipy.io.wavfile as wavfile
import numpy

sample_rate = 228000
samples = numpy.zeros(2 * sample_rate, dtype=numpy.dtype('>i2'))

# 1-second tune
samples[:sample_rate] = (numpy.sin(2*numpy.pi*440*numpy.arange(sample_rate)/sample_rate)
        * 20000).astype(numpy.dtype('>i2'))

wavfile.write("pulses.wav", sample_rate, samples)
