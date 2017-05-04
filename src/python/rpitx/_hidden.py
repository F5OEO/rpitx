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

    dev_null = open(os.devnull, 'w')
    if subprocess.call(('which', 'ffmpeg'), stdout=dev_null) == 0:
        Input = ffmpegwrapper.Input
        Output = ffmpegwrapper.Output
        VideoCodec = ffmpegwrapper.VideoCodec
        AudioCodec = ffmpegwrapper.AudioCodec
        Stream = ffmpegwrapper.FFmpeg
        command = 'ffmpeg'
    elif subprocess.call(('which', 'avconv'), stdout=dev_null) == 0:
        Input = libavwrapper.Input
        Output = libavwrapper.Output
        VideoCodec = libavwrapper.VideoCodec
        AudioCodec = libavwrapper.AudioCodec
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
        output_audio = Output(pipe_name, codec)
        stream = Stream(command, input_media, output_audio)
        # Force overwriting existing file (it's a FIFO, that's what we want)
        if hasattr(stream, 'add_option'):
            stream.add_option('-y', '')
        else:
            stream.add_parameter('-y', '')
        logger.debug(str(stream).replace("'", '').replace(',', ''))
        lines = stream.run()
        for line in lines:
            logger.debug(line)

    thread = threading.Thread(target=convert)
    thread.start()
    logger.debug('Calling broadcast_fm')
    _rpitx.broadcast_fm(pipe_name, frequency)
    thread.join()
