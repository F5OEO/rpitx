#!/bin/sh

convert -depth 8 "$2" picture.rgb
sudo ./pisstv picture.rgb "$1"
