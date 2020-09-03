#!/bin/sh 
#apt-get -y install xsltproc

export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld

find ${ACCSRC}/user/thirdpty/libxslt-1.1.28 -type f -exec chmod 777 '{}' \;

cd ${ACCSRC}/user/thirdpty/libxslt-1.1.28
make distclean

mkdir -p ${ACCPROJ}/user/thirdpty/libxslt-1.1.28
cd ${ACCPROJ}/user/thirdpty/libxslt-1.1.28
make distclean

NOCONFIGURE=1 ${ACCSRC}/user/thirdpty/libxslt-1.1.28/autogen.sh

${ACCSRC}/user/thirdpty/libxslt-1.1.28/configure \
    --build=${BUILD_GNU_TYPE} \
    --host=${HOST_GNU_TYPE} \
    --prefix=${ACCPROJ}/user/thirdpty/libxslt-1.1.28 \
    --with-libxml-libs-prefix=${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0 \
    --with-libxml-prefix=${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0 \
    --with-libxml-include-prefix=${ACCSRC}/user/thirdpty/xml/libxml2-2.9.0/include \
    --with-python=${ACCSRC}/../utility/Python/Versions/2.7

if [ $? != 0 ] ; then
    echo "Error: Configure libxslt."
    exit 1
fi

make install
if [ $? -ne 0 ]; then
   echo "Error: Make install libnetconf."
   exit 1
fi
