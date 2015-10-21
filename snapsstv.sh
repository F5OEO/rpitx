raspistill -w 320 -h 256 -o picture.jpg -t 1
convert -depth 8 picture.jpg picture.rgb

./pisstv picture.rgb picture.ft
sudo ./rpitx -m RF -i picture.ft -f 100000 

