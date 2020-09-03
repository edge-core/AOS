#!/bin/bash

ARCHIVE_NAME=aos_files.tgz
MANIFEST_LOC=usr/share/aos

show_error_and_exit()
{
	echo "Error."
	exit 1
}

set -e

trap 'show_error_and_exit' ERR

[ -f ${ARCHIVE_NAME} ] && rm ${ARCHIVE_NAME}

# Update Manifest
cd rootfs
[ -d ${MANIFEST_LOC} ] || mkdir -p ${MANIFEST_LOC}

find . -type f -o -type l > ${MANIFEST_LOC}/Manifest
tar zcf ../${ARCHIVE_NAME} -T ${MANIFEST_LOC}/Manifest || ( echo "Error" && exit 1 )
cd -

echo "Create ${ARCHIVE_NAME} done."
exit 0
