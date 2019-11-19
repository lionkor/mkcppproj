#!/bin/bash

if [ ! -d "./bin" ] 
then
    echo "bin dir not found, creating..."
    mkdir bin
else
    echo "bin dir found"
fi

clear
cd bin
rm mkcppproj 
cmake -DCMAKE_BUILD_TYPE=Debug ../
make -j 9
cd ..

