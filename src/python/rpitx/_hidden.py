"""Hides imports and other irrelevant things so that ipython works nicely."""

from pydub import AudioSegment
import StringIO
import _rpitx
import array
import logging
import wave


def broadcast_fm(file_, frequency):
    """Play a music file over FM."""

    logging.basicConfig()
    logger = logging.getLogger('rpitx')

    def _reencode(file_name):
        """Returns an AudioSegment file reencoded to the proper WAV format."""
        original = AudioSegment.from_file(file_name)
        if original.channels > 2:
            raise ValueError('Too many channels in sound file')
        if original.channels == 2:
            # TODO: Support stereo. For now, just overlay into mono.
            logger.info('Reducing stereo channels to mono')
            left, right = original.split_to_mono()
            original = left.overlay(right)

        return original

    raw_audio_data = _reencode(file_)

    wav_data = StringIO.StringIO()
    wav_writer = wave.open(wav_data, 'w')
    wav_writer.setnchannels(1)
    wav_writer.setsampwidth(2)
    wav_writer.setframerate(48000)
    wav_writer.writeframes(raw_audio_data.raw_data)
    wav_writer.close()

    raw_array = array.array('c')
    raw_array.fromstring(wav_data.getvalue())
    array_address, length = raw_array.buffer_info()
    _rpitx.broadcast_fm(array_address, length, frequency)
