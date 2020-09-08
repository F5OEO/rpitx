#!/bin/sh

raspistill -w 320 -h 256 -o picture.jpg -t 1
convert -depth 8 picture.jpg picture.rgb

sudo ./pisstv picture.rgb "$1"


