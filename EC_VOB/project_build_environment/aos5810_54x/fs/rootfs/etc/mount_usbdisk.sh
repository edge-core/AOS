#!/bin/sh
USBDISK_MOUNT_PATH='/mnt/usb'

# check whether external usbdisk had already been mounted
grep $USBDISK_MOUNT_PATH /proc/mounts 1>/dev/null
if [ $? -eq "0" ]; then
	exit 0
fi

if [ ! -d $USBDISK_MOUNT_PATH ]; then
    mkdir $USBDISK_MOUNT_PATH
fi

# sourcing usb device utility functions
. /etc/usb_dev_util_functions

# probe board id
if ! probe_board_id ; then
	exit 1
fi

find_ext_usb_disk_dev
if [ "${dev_no}" -eq "0" ]; then
	exit 2
fi

eval dev_name="\$DEV_NAME$dev_no"
(mount -t vfat /dev/${dev_name}1 ${USBDISK_MOUNT_PATH} >/dev/null 2>&1) || \
	(mount -t vfat /dev/${dev_name} ${USBDISK_MOUNT_PATH} > /dev/null 2>&1)

if [ "$?" -ne "0" ]; then
	exit 3
fi

exit 0
