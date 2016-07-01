rm -Rf open62541
git clone https://github.com/open62541/open62541.git
cd open62541
git checkout 7b273f046a0affc61e83837f8df3b6377a835512
rm -Rf build
mkdir build
cd build
cmake -DUA_ENABLE_AMALGAMATION=ON ../
make
cd ../..
rm -f include/open62541.h
ln -s ../open62541/build/open62541.h  include/open62541.h 



