#!/bin/sh

export CC=${CROSS_COMPILE}gcc


find ${ACCSRC}/user/thirdpty/libssh2-1.4.3 -type f -exec chmod 777 '{}' \;

mkdir -p ${ACCPROJ}/user/thirdpty/libssh2-1.4.3
cd ${ACCPROJ}/user/thirdpty/libssh2-1.4.3
#make distclean

${ACCSRC}/user/thirdpty/libssh2-1.4.3/configure \
   --build=${BUILD_GNU_TYPE} \
   --host=${HOST_GNU_TYPE} \
   --prefix=${ACCPROJ}/user/thirdpty/libssh2-1.4.3/out \
   --with-libssl-prefix=${ACCPROJ}/user/thirdpty/openssl/out \
   --with-openssl \
   --without-libz-prefix \
   --disable-examples-build \
   --disable-rpath \
   LDFLAGS=-L${ACCPROJ}/user/thirdpty/openssl/out/lib \
   LIBS='-lssl -lcrypto'
   
if [ $? != 0 ] ; then
    echo "Error: Configure libssh2."
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Error: Make install libssh2."
    exit 1
fi
