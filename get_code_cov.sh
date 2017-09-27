#!/bin/bash
for filename in $(find src/**/*.cpp)
do
  gcov -n -o $(dirname $filename | sed -e 's|src/|src/dbg/|g' ) $filename > /dev/null;
done
