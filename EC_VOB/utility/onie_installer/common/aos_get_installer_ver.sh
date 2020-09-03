#!/bin/sh
# This script file is used to get version from the installer file
# The version of the given installer file will be output if sucesss.
# Return Value:
#   0  - Successfully
#   1  - Incorrect argument number
#   2  - Version string cannot be found in the given installer file
#   3  - The given installer file not exists or not a file
#   4  - The given installer file cannot be read
#   5  - Installer version too old
#   6  - Invalid script version in the installer file
# Note! This file is copied from utility/onie_installer/common/aos_get_installer_ver.sh
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
	echo "Usage: $0 <installer file>"
	exit 1
fi

installer_file=$1

# Check whether the installer file exists and is a file
if [ ! -f ${installer_file} ]; then
	echo "${installer_file} is not a file."
	exit 3
fi

# Check the read permission of the installer file
if [ ! -r ${installer_file} ]; then
	echo "Error! Cannot read ${installer_file}."
	exit 4
fi

# Check the script verion in the installer file
if ! get_script_ver_str_from_installer_file ${installer_file} ; then
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

aos_runtime_ver_line=$(sed -n -e '1,/^exit_marker$/p' ${installer_file} | grep '^aos_runtime_ver=')

if [ -z ${aos_runtime_ver_line} ]; then
	echo "Error! Version string not found."
	exit 2
fi

echo $(echo $aos_runtime_ver_line|sed 's/^aos_runtime_ver=//')

exit 0
