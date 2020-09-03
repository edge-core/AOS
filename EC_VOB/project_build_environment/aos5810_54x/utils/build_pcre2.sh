#!/bin/bash

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld

# Have no idea to make the obj files into builddir. So copy all files to builddir.
cp -r ${ACCSRC}/user/thirdpty/pcre2 ${ACCPROJ}/user/thirdpty/pcre2

find ${ACCPROJ}/user/thirdpty/pcre2 -type f -exec chmod 777 '{}' \;

cd ${ACCPROJ}/user/thirdpty/pcre2

test -d ${ACCPROJ}/user/thirdpty/pcre2/out || mkdir ${ACCPROJ}/user/thirdpty/pcre2/out

autoreconf --force --install

./configure \
CC=${CROSS_COMPILE}gcc \
AR=${CROSS_COMPILE}ar \
LD=${CROSS_COMPILE}ld \
    --host=${HOST_GNU_TYPE} \
    --prefix=${ACCPROJ}/user/thirdpty/pcre2/out

make && make install

install -m 755 ${ACCPROJ}/user/thirdpty/pcre2/out/lib/libpcre2-8.so.0 ${ACCROOTFS}/usr/lib/
install -m 755 ${ACCPROJ}/user/thirdpty/pcre2/out/lib/libpcre2-8.so.0.2.0 ${ACCROOTFS}/usr/lib/

cd ${ACCROOTFS}/usr/lib
ln -fs libpcre2-8.so.0.2.0 libpcre2-8.so.0

${CROSS_COMPILE}strip libpcre2-8.so.0.2.0
