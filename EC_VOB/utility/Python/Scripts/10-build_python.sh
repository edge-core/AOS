#!/usr/bin/env bash

PREFIX=${1:-"."}

test -d "${PREFIX}" || mkdir -p "${PREFIX}"
cd "${PREFIX}"
PREFIX=`pwd`
cd -

TOP_DIR=$(cd $(dirname "${BASH_SOURCE:-$0}") && pwd)

source $TOP_DIR/functions

##Parameters

PYTHON_MAJOR_VERSION=2
PYTHON_MINOR_VERSION=7
PYTHON_MIRCO_VERSION=8

###PYTHON_VERSION (e.g., 2.7.8)
PYTHON_VERSION="$PYTHON_MAJOR_VERSION.$PYTHON_MINOR_VERSION.$PYTHON_MIRCO_VERSION"

###PYTHON_DESTDIR (e.g., Versions/2.7)
PYTHON_DESTDIR="$PREFIX/Versions/$PYTHON_MAJOR_VERSION.$PYTHON_MINOR_VERSION"

###PYTHON_URL (e.g., http://python.org/ftp/python/2.7.8/)
PYTHON_URL="http://python.org/ftp/python/$PYTHON_VERSION"

###PYTHON_TGZ_FILENAME (e.g., Python-2.7.8.tgz)
PYTHON_TGZ_FILENAME=Python-$PYTHON_VERSION.tgz

###PYTHON_EXTRACTDIR (e.g., Python-2.7.8)
PYTHON_EXTRACTDIR=Python-$PYTHON_VERSION

##Prepare directory

test -d "$PYTHON_DESTDIR" || mkdir -p "$PYTHON_DESTDIR"
test -d "$PYTHON_DESTDIR/lib" || mkdir -p "$PYTHON_DESTDIR/lib"
test -d "$PYTHON_DESTDIR/Sources" || mkdir -p "$PYTHON_DESTDIR/Sources"

##Download, compile and install bzip2
#
cd "$PYTHON_DESTDIR/Sources"
test -f bzip2-1.0.6.tar.gz || wget http://bzip.org/1.0.6/bzip2-1.0.6.tar.gz
tar xpzf bzip2-1.0.6.tar.gz
cd - >/dev/null
#
cd "$PYTHON_DESTDIR/Sources/bzip2-1.0.6"
make -f Makefile-libbz2_so
make install PREFIX=$PYTHON_DESTDIR
cd - >/dev/null

##Download, compile and install Python...

###Download source to Versions/2.7/Sources
cd "$PYTHON_DESTDIR/Sources"
test -f "$PYTHON_TGZ_FILENAME" || wget $PYTHON_URL/$PYTHON_TGZ_FILENAME
tar zxvf "$PYTHON_TGZ_FILENAME"
cd - >/dev/null

cp -f "$PYTHON_DESTDIR/Sources/$PYTHON_EXTRACTDIR/Modules/Setup.dist" "$PYTHON_DESTDIR/Sources/$PYTHON_EXTRACTDIR/Modules/Setup"

###Compile and install at Versions/2.7/Sources/Python-2.7.8
cd "$PYTHON_DESTDIR/Sources/$PYTHON_EXTRACTDIR"
#./configure --prefix=$PYTHON_DESTDIR --enable-unicode=ucs4 --enable-shared LDFLAGS="-Wl,-rpath $PYTHON_DESTDIR/lib"
./configure --prefix=$PYTHON_DESTDIR --enable-unicode=ucs4 --disable-shared
make && make altinstall
cd - >/dev/null

###Making the symbolic link, Version/2.7/bin/python -> python2.7
test -e "$PYTHON_DESTDIR/bin/python" && rm "$PYTHON_DESTDIR/bin/python"
ln -s "python$PYTHON_MAJOR_VERSION.$PYTHON_MINOR_VERSION" "$PYTHON_DESTDIR/bin/python"

###Making the symbolic link, Version/Current -> 2.7
test -e "$PREFIX/Versions/Current" && rm "$PREFIX/Versions/Current"
ln -fs "$(compute_relative $PREFIX/Versions $PYTHON_DESTDIR)" "$PREFIX/Versions/Current"

###Cleaning download stuffs
rm -rf "$PYTHON_DESTDIR/Sources"
