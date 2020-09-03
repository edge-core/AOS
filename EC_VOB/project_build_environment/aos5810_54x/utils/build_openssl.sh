#!/bin/sh

export CC=gcc
export AR=ar
export LD=ld

cp -r ${ACCSRC}/user/thirdpty/openssl ${ACCPROJ}/user/thirdpty/openssl

find ${ACCPROJ}/user/thirdpty/openssl -type f -exec chmod 777 '{}' \;

cd ${ACCPROJ}/user/thirdpty/openssl
perl Configure linux-x86_64 shared --prefix=${ACCPROJ}/user/thirdpty/openssl/out \
no-md2 no-md4 no-mdc2 no-ripemd \
no-rc2 no-rc5 no-idea no-camellia \
no-seed no-cast \
no-asm \
no-krb5 \
no-rfc3779 no-hw no-engine \
-DOPENSSL_NO_DTLS1

make dclean
make
make install_sw

install -m 755 libcrypto.so.0.9.8 ${ACCROOTFS}/usr/lib
install -m 755 libssl.so.0.9.8 ${ACCROOTFS}/usr/lib

cd ${ACCROOTFS}/usr/lib
ln -fs libcrypto.so.0.9.8 libcrypto.so
ln -fs libssl.so.0.9.8 libssl.so

cp libcrypto.so ${ACCPROJ}/user/thirdpty/openssl
cp libssl.so ${ACCPROJ}/user/thirdpty/openssl

