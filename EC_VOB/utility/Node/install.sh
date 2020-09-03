#!/usr/bin/env bash

PREFIX=${1:-"."}

test -d "${PREFIX}" || mkdir -p "${PREFIX}"
cd "${PREFIX}"
PREFIX=`pwd`
cd -

TOP_DIR=$(cd $(dirname "${BASH_SOURCE:-$0}") && pwd)

source $TOP_DIR/functions

##Parameters

NODE_MAJOR_VERSION=0
NODE_MINOR_VERSION=10
NODE_MIRCO_VERSION=33

###NODE_VERSION (e.g., 0.10.33)
NODE_VERSION="$NODE_MAJOR_VERSION.$NODE_MINOR_VERSION.$NODE_MIRCO_VERSION"

###NODE_DESTDIR (e.g., Versions/0.10)
NODE_DESTDIR="$PREFIX/Versions/$NODE_MAJOR_VERSION.$NODE_MINOR_VERSION"

###NODE_URL (e.g., http://nodejs.org/dist/v0.10.33/)
NODE_URL="http://nodejs.org/dist/v$NODE_VERSION"

###NODE_TGZ_FILENAME (e.g., node-v0.10.33-linux-x64.tar.gz)
NODE_TGZ_FILENAME=node-v$NODE_VERSION-linux-x64.tar.gz

###NODE_EXTRACTDIR (e.g., node-v0.10.33-linux-x64)
NODE_EXTRACTDIR=node-v$NODE_VERSION-linux-x64

## Download and install Node

test -f $NODE_TGZ_FILENAME || wget $NODE_URL/$NODE_TGZ_FILENAME
tar zxvf $NODE_TGZ_FILENAME

test -d "$PREFIX/Versions" || mkdir -p "$PREFIX/Versions"
test -d "$NODE_DESTDIR" && rm -rf "$NODE_DESTDIR"

###Move node-v0.10.33-linux-x64 to Versions/0.10
mv $NODE_EXTRACTDIR "$NODE_MAJOR_VERSION.$NODE_MINOR_VERSION"
mv "$NODE_MAJOR_VERSION.$NODE_MINOR_VERSION" "$PREFIX/Versions"

###Making the symbolic link, Version/Current -> 0.10
test -e "$PREFIX/Versions/Current" && rm "$PREFIX/Versions/Current"
ln -s "$(compute_relative $PREFIX/Versions $NODE_DESTDIR)" "$PREFIX/Versions/Current"

## Remove downloaded files
rm $NODE_TGZ_FILENAME
