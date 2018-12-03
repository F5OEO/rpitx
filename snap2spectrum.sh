#!/bin/sh

raspistill -w 320 -h 256 -o picture.jpg -t 1
#convert  picture.jpg -flip -colors 16 -colorspace gray -dither -colorspace YUV picture.yuv
#convert picture.jpg -flip -colors 16 -colorspace gray -colorspace YUV picture.yuv
#convert BBC-Test-Card-F320x256.jpg -flip -quantize YUV -dither FloydSteinberg -colors 4 -interlace partition picture.yuv
convert picture.jpg -flip -quantize YUV -dither FloydSteinberg -colors 4 \
  -interlace partition picture.yuv
sudo ./spectrumpaint picture.Y "$1" 100000

#convert -depth 8 picture.jpg picture.rgb

#sudo ./pisstv picture.rgb 144.5e6


