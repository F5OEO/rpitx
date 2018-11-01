raspistill -w 320 -h 256 -o picture.jpg -t 1
#convert  picture.jpg -flip -colors 16 -colorspace gray -dither -colorspace YUV picture.yuv
convert picture.jpg -flip -colors 16 -colorspace gray -colorspace YUV picture.yuv
sudo ./spectrumpaint picture.yuv 144.2e6

#convert -depth 8 picture.jpg picture.rgb

#sudo ./pisstv picture.rgb 144.5e6


