#!/bin/bash

TMP_ROOTFS_DIR=rootfs_tmp
INSTALLED_ROOTFS_DIR=rootfs
BASE_ROOTFS_ARCHIVE=initrd_base.gz

cp ${PROJECT_PATH}/initramfs/initrd.cpio.gz ${BASE_ROOTFS_ARCHIVE}


if [ -d ${TMP_ROOTFS_DIR} ]; then
	rm -rf ${TMP_ROOTFS_DIR}
fi

mkdir -p ${TMP_ROOTFS_DIR} || exit 1

cd ${TMP_ROOTFS_DIR}
gzip -cd ../${BASE_ROOTFS_ARCHIVE} | cpio -i
if [ $? -ne 0 ]; then
	echo "Failed to extract archive file '${BASE_ROOTFS_ARCHIVE}'"
	exit 1
fi

cd ../${INSTALLED_ROOTFS_DIR} || exit 1

find . -type f -o -type l | grep -v '.gitignore' | cpio -p -d ../${TMP_ROOTFS_DIR}

if [ $? -ne 0 ]; then
	echo "Failed to copy files under ${INSTALLED_ROOTFS_DIR} to ${TMP_ROOTFS_DIR}"
	exit 1
fi

cp ${PROJECT_PATH}/initramfs/init ${PROJECT_PATH}/fs/${TMP_ROOTFS_DIR}/init

cd .. && ./mkinitramfs.sh ${TMP_ROOTFS_DIR} initrd.gz

exit 0

