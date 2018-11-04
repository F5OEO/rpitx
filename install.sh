
echo Install rpitx - some package need internet connection -
sudo apt-get update
sudo apt-get install -y libsndfile1-dev
sudo apt-get install -y imagemagick
sudo mknod /dev/rpidatv-mb c 100 0
cd src
make -j4
sudo make install
cd ..
echo Installation done
