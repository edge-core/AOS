#!/bin/sh
# This script file is used to verify the installer file
# Return Value:
#   0  - Successfully
#   1  - Incorrect arguments
#   2  - Failed to access the specified installer file
#   3  - The permission of the specified installer file is incorrect
#   4  - Installer returns error
#   5  - Installer version too old
#   6  - Invalid script version in the installer file
#
# Note! This file is copied from utility/onie_installer/common/aos_verify_installer.sh
# to $ACCROOTFS/etc in the script "8_build_rootfs"
#
. /etc/aos_util_functions

# When AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL is 10000, it means
# that this script is capable of handling the installer file that
# contains the install script file with aos_installer_script_ver >= 1.0.0
# Refer to function get_script_ver_str_from_installer_file in the file
# "aos_util_functions" for details of the version string format and version value.
# "%%AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL%%" below shall be replaced by the real value.
AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL=%%AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL%%

# Check argument number
if [ $# -ne 1 ]; then
	echo "Usage: $0 <full path to installer file>"
	exit 1
fi

installer_file_full_path=$1

# Check whether the installer file exists and is a file
if [ ! -f ${installer_file_full_path} ]; then
	echo "Error! ${installer_file_full_path} is not a file."
	exit 2
fi

# Check the read permission of the installer file
if [ ! -r ${installer_file_full_path} -o ! -x ${installer_file_full_path} ]; then
	echo "Error! Cannot read or execute ${installer_file_full_path}."
	exit 3
fi

# Check the script verion in the installer file
if ! get_script_ver_str_from_installer_file ${installer_file_full_path} ; then
	echo "Error! The installer version is too old."
	exit 5
fi

if ! ver_str_to_val $FUNC_OUTPUT_VER_STR ; then
	echo "Error! Invalid script version in the installer file."
	exit 6
fi

if [ $VERSION_VAL -lt $AOS_INSTALLER_SCRIPT_COMPAT_VER_VAL ]; then
	echo "Error! The installer version is too old."
	exit 5
fi

export aos_do_validation_only=1
export aos_installer_filename=$(basename $installer_file_full_path)
${installer_file_full_path} ${installer_file_full_path}

if [ $? -ne 0 ]; then
	echo "Error! Installer returns error."
	exit 4
fi

exit 0
