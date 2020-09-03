#!/bin/sh
# This script file is used to change the startup installer file
# under the directory /flash/.fs
# Return Value:
#   0  - Successfully
#   1  - Incorrect arguments
#   2  - The directory "/flash/.fs" not exists
#   3  - Failed to access the specified installer file
#   4  - The permission of the specified installer file is incorrect
#   5  - Installer returns error
#   6  - Installer version too old
#   7  - Invalid script version in the installer file
#
# Note! This file is copied from utility/onie_installer/common/aos_change_startup_installer.sh
# to $ACCROOTFS/etc in the script "8_build_rootfs"
#

. /etc/aos_util_functions
AOS_FS_TOP_DIR=/flash/.fs
# When AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL is 10000, it means
# that this script is capable of handling the installer file that
# contains the install script file with aos_installer_script_ver >= 1.0.0
# Refer to function get_script_ver_str_from_installer_file in the file
# "aos_util_functions" for details of the version string format and version value.
# "%%AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL%%" below shall be replaced by the real value.
AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL=%%AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL%%

# Check argument number
if [ $# -lt 1 ]; then
	echo "Usage: $0 <installer filename in '$AOS_FS_TOP_DIR'>"
	exit 1
fi

installer_file=$1

cd $AOS_FS_TOP_DIR
if [ $? -ne 0 ]; then
    echo "Error! Failed to change working directory to '$AOS_FS_TOP_DIR'"
    exit 2
fi

# Check whether the installer file exists and is a file
if [ ! -f ${installer_file} ]; then
	echo "Error! ${installer_file} is not a file."
	exit 3
fi

# Check the read permission of the installer file
if [ ! -r ${installer_file} -o ! -x ${installer_file} ]; then
	echo "Error! Cannot read or execute ${installer_file}."
	exit 4
fi

# Check the script verion in the installer file
if ! get_script_ver_str_from_installer_file $AOS_FS_TOP_DIR/${installer_file} ; then
	echo "Error! The installer version is too old."
	exit 6
fi

if ! ver_str_to_val $FUNC_OUTPUT_VER_STR ; then
	echo "Error! Invalid script version in the installer file."
	exit 7
fi

if [ $VERSION_VAL -lt $AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL ]; then
	echo "Error! The installer version is too old."
	exit 6
fi

if [ install_when_change_startup = 1 ]; then
	export aos_skip_copy_installer=1
	export aos_installer_filename=${installer_file}
	export aos_root=${root}
	$AOS_FS_TOP_DIR/${installer_file} $AOS_FS_TOP_DIR/${installer_file}
else
	CurrentBoot=/usr/share/aos/CurrentBoot
	echo $installer_file > $CurrentBoot
	sync; sync
fi


if [ $? -ne 0 ]; then
	echo "Error! Installer returns error."
	exit 5
fi

exit 0
