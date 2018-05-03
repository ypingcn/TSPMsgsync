sudo apt install git -y
cd ../include
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo cp libhiredis.so /usr/lib/libhiredis.so.0.13
