#!/bin/bash

D="rawData"

read -p "Do you want to start the compression? This will unzip the entries of rawData and recompress them using xz"

echo "unzipping the contents of $D"
cd "$D"
gunzip *.gz

echo "recompressing $D"
cd ..
tar -cJf "$D.tar.xz" "$D"
