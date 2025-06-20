cd ~/libiio
mkdir build
cd build
cmake -DPYTHON_BINDINGS=ON .. 
make 
sudo make install 
sudo ldconfig
