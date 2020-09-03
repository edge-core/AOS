#!/bin/bash

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld

VERSION=1.1.10
BUILDDIR=${ACCPROJ}/user/thirdpty/libbson-${VERSION}.build.dir
PREFIX=${ACCPROJ}/user/thirdpty/libbson-${VERSION}

# Have no idea to make the obj files into builddir. So copy all files to builddir.
cp -r ${ACCSRC}/user/thirdpty/libbson-${VERSION} ${BUILDDIR}

find ${BUILDDIR} -type f -exec chmod +x '{}' \;

cd ${BUILDDIR}

./autogen.sh

./configure \
    --host=${HOST_GNU_TYPE} \
    --prefix=${PREFIX}

make && make install

install -m 755 ${PREFIX}/lib/libbson-1.0.so.0 ${ACCROOTFS}/usr/lib/
install -m 755 ${PREFIX}/lib/libbson-1.0.so.0.0.0 ${ACCROOTFS}/usr/lib/

cd ${ACCROOTFS}/usr/lib
ln -fs libbson-1.0.so.0.0.0 libbson-1.0.so.0

${CROSS_COMPILE}strip libbson-1.0.so.0.0.0
