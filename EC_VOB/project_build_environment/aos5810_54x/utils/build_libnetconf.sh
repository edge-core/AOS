#!/bin/sh

export CC=${CROSS_COMPILE}gcc


find ${ACCSRC}/user/thirdpty/libnetconf -type f -exec chmod 777 '{}' \;

mkdir -p ${ACCPROJ}/user/thirdpty/libnetconf
cd ${ACCPROJ}/user/thirdpty/libnetconf
make distclean
make clean

${ACCSRC}/user/thirdpty/libnetconf/configure \
    --build=${BUILD_GNU_TYPE} \
    --host=${HOST_GNU_TYPE} \
    --prefix=${ACCPROJ}/user/thirdpty/libnetconf \
    --with-libssh2=${ACCPROJ}/user/thirdpty/libssh2-1.4.3/out \
    --with-libxslt=${ACCPROJ}/user/thirdpty/libxslt-1.1.28/bin \
    --with-libxml2=${ACCPROJ}/user/thirdpty/xml/libxml2-2.9.0 \
    --with-workingdir=${ACCROOTFS}/var/lib/libnetconf \
    --disable-url \
    LDFLAGS=-L${ACCPROJ}/user/thirdpty/openssl/out/lib \
    LIBS='-lssl -lcrypto'
	
if [ $? != 0 ] ; then
    echo "Error: Configure libnetconf."
    exit 1
fi

make

if [ $? -ne 0 ]; then
    echo "Error: Make libnetconf."
    exit 1
fi

# need to do make install to install the generated files
# to the dirctory under $ACCPROJ/user/thirdpty/libnetconf
# For now, this step would emit error regarding lnctool.
# The error seems to be related to lack of pyang package
# in Python. Fix this error if we need them.
make install 1>/dev/null 2>&1
exit 0

#cd ${ACCPROJ}/user/thirdpty/libnetconf
#cd ${ACCROOTFS}/usr/lib
#ln -fs libxml2.so.2.9.0 libxml2.so
#ln -fs libxml2.so.2.9.0 libxml2.so.2
