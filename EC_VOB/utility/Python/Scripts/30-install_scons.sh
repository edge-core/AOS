#!/usr/bin/env bash

PYTHON=${1:-"$(pwd)/Versions/Current/bin/python"}

test -f scons-2.3.4.tar.gz || wget http://prdownloads.sourceforge.net/scons/scons-2.3.4.tar.gz

tar zxvf scons-2.3.4.tar.gz

cd scons-2.3.4
$PYTHON setup.py install
cd -

rm scons-2.3.4.tar.gz
rm -rf scons-2.3.4
