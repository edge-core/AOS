#!/usr/bin/env bash

PYTHON=${1:-"$(pwd)/Versions/Current/bin/python"}
EASY_INSTALL=${2:-"$(pwd)/Versions/Current/bin/easy_install"}

## install Setuptools + pip

# First get the setup script for Setuptools:
test -f ez_setup.py || wget https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py

# Then install it for Python 2.7
$PYTHON ez_setup.py

# Now install pip using the newly installed setuptools
$EASY_INSTALL pip

rm ez_setup.py
rm setuptools-*.zip
