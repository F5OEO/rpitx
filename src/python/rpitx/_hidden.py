"""Hides imports and other irrelevant things so that ipython works nicely."""

import logging
import os
import subprocess
import sys
import threading
import _rpitx
import ffmpegwrapper
import libavwrapper

PIPE = 'pipe:1'
DUMMY_FILE = 'dummy-file.wav'


def broadcast_fm(media_file_name, frequency):
    """Broadcast a media file's audio over FM.
Args:
    media_file_name (str): The file to broadcast.
    frequency (float): The frequency, in MHz, to broadcast on."""

    logging.basicConfig()
    logger = logging.getLogger('rpitx')

    # Python < 3.3 throws IOError instead of PermissionError
    if sys.version_info.major <= 2 or (sys.version_info.major == 3 and sys.version_info.minor < 3):
        Error = IOError
    else:
        Error = PermissionError

    # mailbox.c calls exit if /dev/mem can't be opened, which causes the avconv
    # pipe to be left open and messes with the terminal, so try it here first
    # just to be safe
    try:
        open('/dev/mem')
    except Error:
        raise Error(
                'This program should be run as root. Try prefixing command with: sudo'
                )

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
        Stream.add_option = Stream.add_parameter
        command = 'avconv'
    else:
        raise NotImplementedError(
            'Broadcasting audio requires either avconv or ffmpeg to be installed\n'
            'sudo apt install libav-tools'
        )

    logger.debug('Using %s', command)

    input_media = Input(media_file_name)
    codec = AudioCodec('pcm_s16le').frequence(48000).channels(1)
    output_audio = Output(DUMMY_FILE, codec)
    stream = Stream(command, input_media, output_audio)
    command_line = list(stream)
    # The format needs to be specified manually because we're writing to
    # stderr and not a named file. Normally, it would infer the format from
    # the file name extension. Also, ffmpeg will only write to a pipe if it's
    # the last named argument.
    command_line.remove(DUMMY_FILE)
    command_line += ('-f', 'wav', PIPE)
    logger.debug('Running command "%s"', ' '.join(command_line))
    stream_process = subprocess.Popen(
            command_line,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)

    def log_stdout():
        """Log output from the stream process."""
        for line in stream_process.stderr:
            logger.debug(line.rstrip())

    thread = threading.Thread(target=log_stdout)
    thread.start()

    logger.debug('Calling broadcast_fm')
    try:
        _rpitx.broadcast_fm(stream_process.stdout.fileno(), frequency)
    except Exception as exc:
        stream_process.stdout.close()
        thread.join()
        raise exc

    thread.join()
