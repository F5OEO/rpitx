#!/bin/sh

convert -depth 8 BBC.jpg picture.rgb
sudo ./pisstv picture.rgb "$1"
