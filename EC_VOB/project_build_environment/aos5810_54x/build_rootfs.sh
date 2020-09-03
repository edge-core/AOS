#!/bin/sh
#              -V <x.x.x.x>  -> specify runtime version number

ROOTFS_FILENAME=initrd.gz

if ! utils/check_build_env; then
	cd utils/;
	. build_env_init;
	cd -
fi

while getopts "mV:" opts
do
    case "$opts" in
        V) RUNTIME_VER=$OPTARG ;;
        *) ;;
    esac
done

if [ "y" = "y"$RUNTIME_VER ]; then
    RUNTIME_VER=`date +%m.%d.%H.%M`
fi

echo "$RUNTIME_VER" > ${ACCROOTFS}/etc/runtime_ver

echo "Making rootfs ..."

rm ${ACCPROJ}/initramfs/initrd.cpio.gz 2>/dev/null
./build_initramfs

cd ${ACCPROJ}/utils
./do_sym.sh
./do_strip.sh

cd ${ACCPROJ}/fs
./pack_aos_files.sh

#rm ${ROOTFS_FILENAME} 2>/dev/null

#./create_initrd.sh
#if [ $? -ne 0 ]; then
#	echo "An error occurred while creating ${ROOTFS_FILENAME}"
#	exit 1
#fi

echo "Done"
exit 0
