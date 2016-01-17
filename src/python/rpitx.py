"""Python interface to rpitx."""

from pydub import AudioSegment
import StringIO
import _rpitx
import array
import logging
import os


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

        if (
                original.frame_rate != 48000
                # TODO: There should be a better way to check if it's wav
                or not file_name.endswith('.wav')
        ):
            logger.debug('Reencoding file')
            reencoded = StringIO.StringIO()
            original.export(reencoded, format='wav', bitrate='44k')
            return reencoded

        return original

    encoded_file = _reencode(file_)
    raw_array = array.array('c')
    raw_array.fromstring(encoded_file.raw_data)
    array_address, length = raw_array.buffer_info()
    _rpitx.broadcast_fm(array_address, length, frequency)
