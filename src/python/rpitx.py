"""Python interface to rpitx."""

from pydub import AudioSegment
import StringIO
import array
import _rpitx
import wave


def play_fm(file_, frequency):
    """Play a music file over FM."""

    def _reencode(file_name):
        """Returns a file-like object reencoded to the proper WAV format."""
        reencoded = StringIO.StringIO()
        # AudioSegment doesn't support context managers either
        original = AudioSegment.from_file(file_name)
        if original.channels > 2:
            raise ValueError('Too many channels in sound file')
        if original.channels == 2:
            # TODO: Support stereo. For now, just overlay into mono.
            left, right = original.split_to_mono()
            original = left.overlay(right)

        original.export(reencoded, format='wav', bitrate='44k')
        return reencoded

    encoded_file = None
    if isinstance(file_, str):
        if file_.endswith('.wav'):
            with open(file_) as raw_file:
                # wave.open doesn't support context managers, so we need to be
                # careful about closing the file
                wav_file = wave.open(raw_file, 'r')
                num_channels = wav_file.getnchannels()
                framerate = wav_file.getframerate()
                sample_width = wav_file.getsampwidth()
                wav_file.close()

                if (
                        num_channels != 1
                        or framerate != 1
                        or sample_width != 2
                ):
                    encoded_file = _reencode(file_)
                else:
                    encoded_file = AudioSegment.from_file(file_)
        else:
            encoded_file = _reencode(file_)
    else:
        encoded_file = _reencode(file_)

    raw_array = array.array('c')
    raw_array.fromstring(str(encoded_file))
    array_address, length = raw_array.buffer_info()
    _rpitx.broadcast(array_address, length, frequency)
