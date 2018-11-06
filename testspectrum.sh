convert BBC.jpg -flip -quantize YUV -dither FloydSteinberg -colors 4 -interlace partition picture.yuv
sudo ./spectrumpaint picture.Y 434.0e6 100000
