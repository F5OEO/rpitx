
echo Install rpitx - some package need internet connexion -
sudo apt-get update
sudo apt-get install  libsndfile1-dev
sudo apt-get install imagemagick
sudo mknod /dev/rpidatv-mb c 100 0
cd src
make -j4
sudo make install
cd ..
echo Installation done
