rm -Rf open62541
git clone https://github.com/open62541/open62541.git
cd open62541
git checkout 0.2-rc2
rm -Rf build
mkdir build
cd build
cmake -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_METHODCALLS=ON  ../
make
cd ../..
rm -f include/open62541.h
ln -s ../open62541/build/open62541.h  include/open62541.h 



