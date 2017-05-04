"""Hides imports and other irrelevant things so that ipython works nicely."""

import _rpitx
import ffmpegwrapper
import libavwrapper
import logging
import os
import subprocess
import threading


def broadcast_fm(media_file_name, frequency):
    """Play a media file's audio over FM."""

    logging.basicConfig()
    logger = logging.getLogger('rpitx')

    if subprocess.call(('which', 'ffmpeg')) == 1:
        Input = ffmpegwrapper.Input
        Output = ffmpegwrapper.Output
        VideoCodec = ffmpegwrapper.VideoCodec
        AudioCoded = ffmpegwrapper.AudioCodec
        Stream = ffmpegwrapper.FFmpeg
        command = 'ffmpeg'
    elif subprocess.call(('which', 'avconv')) == 1:
        Input = libavwrapper.Input
        Output = libavwrapper.Output
        VideoCodec = libavwrapper.VideoCodec
        AudioCoded = libavwrapper.AudioCodec
        Stream = libavwrapper.AVConv
        command = 'avconv'
    else:
        raise NotImplementedError(
            'Broadcasting audio requires either avconv or ffmpeg to be installed'
        )

    logger.debug('Using {}'.format(command))

    pipe_name = '/tmp/rpitx-fifo.wav'
    if not os.path.exists(pipe_name):
        os.mkfifo(pipe_name)

    def convert():
        """Runs the conversion command and writes to a FIFO."""
        input_media = Input(media_file_name)
        codec = AudioCodec('pcm_s16le').frequence(48000).channels(1)
        output_audio = Output(pipe_name, pipe_out)
        stream = Stream(command, input_media, output_audio)
        stream.add_option('-y', '')  # Force overwriting existing file
        logger.debug('Extracting and converting audio')
        stream.run()

    thread = threading.Thread(target=convert)
    thread.start()
    logger.debug('Calling broadcast_fm')
    _rpitx.broadcast_fm(pipe_name, frequency)
    thread.join()
