#!/bin/sh

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld

find ${ACCSRC}/user/thirdpty/xml/libxml2-2.9.0 -type f -exec chmod 777 '{}' \;

if [ ! -d ${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0 ]; then
    mkdir -p ${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0
fi

[ ! -d ${ACCROOTFS}/usr/lib ] && mkdir -p ${ACCROOTFS}/usr/lib

cd ${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0
make distclean

${ACCSRC}/user/thirdpty/xml/libxml2-2.9.0/configure \
    --build=${BUILD_GNU_TYPE} \
    --host=${HOST_GNU_TYPE} \
    --prefix=${PWD} \
    --enable-ipv6=no \
    --with-c14n=no \
    --with-docbook=no \
    --with-ftp=no \
    --without-iconv \
    --with-iso8859x=no \
    --with-legacy=no \
    --with-push=no \
    --without-python \
    --with-reader=yes \
    --without-readline \
    --with-threads=no \
    --with-valid=no \
    --with-writer=no \
    --without-zlib
if [ $? != 0 ] ; then
    echo "Error: Configure LibXML."
    exit 1
fi

make
if [ $? != 0 ] ; then
    echo "Error: Make LibXML."
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Error: Make install LibXML."
    exit 1
fi

cd ${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0


install -m 755 lib/libxml2.so.2.9.0 ${ACCROOTFS}/usr/lib
cd ${ACCROOTFS}/usr/lib
ln -fs libxml2.so.2.9.0 libxml2.so
ln -fs libxml2.so.2.9.0 libxml2.so.2
