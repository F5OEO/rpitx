
echo Install rpitx - some package need internet connexion -
sudo apt-get update
sudo apt-get install -y libsndfile1-dev git
sudo apt-get install -y imagemagick
cd src
git clone https://github.com/F5OEO/librpitx
cd librpitx/src
make
cd ../../
make 
sudo make install
cd ..
echo Installation done
