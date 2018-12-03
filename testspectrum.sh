#!/bin/sh

convert BBC.jpg -flip -quantize YUV -dither FloydSteinberg -colors 4 \
  -interlace partition picture.yuv
sudo ./spectrumpaint picture.Y "$1" 100000
