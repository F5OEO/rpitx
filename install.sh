
echo Install rpitx - some package need internet connexion -
sudo apt-get update
sudo apt-get install -y libsndfile1-dev git
sudo apt-get install -y imagemagick libfftw3-dev
# We use CSDR as a dsp for analogs modes thanks to HA7ILM
git clone https://github.com/simonyiszk/csdr
cd csdr
make && sudo make install
cd ../

cd src
git clone https://github.com/F5OEO/librpitx
cd librpitx/src
make
cd ../../
make 
sudo make install
cd ..
echo Installation done
