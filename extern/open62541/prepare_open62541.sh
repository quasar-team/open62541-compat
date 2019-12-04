#!/bin/bash

# the purpose of this script is to make the process of deploying open62541 in open62541-compat
# automated, reproducible

# this script was created to be used only by quasar-developers team.

TAG=v0.3.1

# prepare the dir structure
mkdir open62541
mkdir open62541/include
mkdir open62541/src

# prepare (i.e. amalgamate... open62541)
WD=`pwd`
if [ -d tmp ]; then
  rm -frv tmp
fi
mkdir tmp && cd tmp
git clone https://github.com/open62541/open62541.git --depth=1 -b $TAG || exit
cd open62541
mkdir build && cd build
cmake -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_METHODCALLS=ON ../ || exit
make || exit
cd $WD

# deploy files
find tmp -name open62541.h -ok cp {} open62541/include \; || exit
find tmp -name open62541.c -ok cp {} open62541/src \; || exit

read -n 1 -p "Would you like to commit the freshly amalgamated open62541? type y if so." answer
if [ $answer == "y" ]; then
  git add open62541/include/open62541.h
  git add open62541/src/open62541.c
  git commit --author="open62541 <open62541@open62541.org>" -m "Amalgamated open62541 v $TAG by prepare_open62541.sh"
fi

# clean-up
rm -fr tmp
