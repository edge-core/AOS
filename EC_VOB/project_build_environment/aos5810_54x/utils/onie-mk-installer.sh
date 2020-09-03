#!/bin/sh

# AOS_INSTALLER_SCRIPT_VER history:
#     Version  Description
# ------------------------------------
#       1.0.0  Initial version that supports to install the ONIE installer
#              file from AOS UI(CLI/Web/Snmp).
#
#       1.1.0  Remove the operation to generate ".fmapping" through fmapgen.
#              Redefine the mapping between uid and attribute(e.g. startup
#              and file type) used in FS. This version is incompatible to
#              1.0.0.
AOS_INSTALLER_SCRIPT_VER=1.1.0

# arg1 - kernel image file to be packed into the installer file
# arg2 - initrd image file to be packed into the installer file
# arg3 - aos file to be packed into the installer file
# arg4 - debian rootfs addon file to be extracted when running installer
# arg5 - debian rootfs file to be packed into the installer file. This file will
#        not be packed when the file does not exist.
# arg6 - ouput file name(installer file)

if [ $# -ne 6 ]; then
    echo "usage:$0 <kernel image file> <initrd file> <aos file> <debian rootfs addon file> <debian rootfs file> <output filename>"
    exit 1
fi

support_machines="as5812_54x as5812_54t as6812_32x"
# dhcp vendor class used by onie: "onie_vendor:${onie_platform}"
installer_dir=$ACCPROJ/utils/onie_installer
common_dir=$UTILITY_PATH/onie_installer/common
aos_runtime_image_ver=`cat $ACCROOTFS/etc/runtime_ver`
kernel_file=$1; shift
initrd_file=$1; shift
aos_file=$1; shift
debian_rootfs_addon_file=$1; shift
debian_rootfs_file=$1; shift
output_file=$1; shift

if  [ ! -d $installer_dir ] || \
    [ ! -r $installer_dir/install.sh ] ; then
    echo "Error: Invalid installer script directory: $installer_dir"
    exit 1
fi

if  [ ! -d $common_dir ] || \
    [ ! -r $common_dir/sharch_body.sh ] ; then
    echo "Error: Invalid installer script directory: $common_dir"
    exit 1
fi

[ -r "$kernel_file" ] || {
    echo "Error: Unable to read kernel image file: $kernel_file"
    exit 1
}

[ -r "$initrd_file" ] || {
    echo "Error: Unable to read initrd image file: $initrd_file"
    exit 1
}

[ -r "$aos_file" ] || {
    echo "Error: Unable to read aos file: $aos_file"
    exit 1
}

[ -r "$debian_rootfs_addon_file" ] || {
    echo "Error: Unable to read debian rootfs addon file: $debian_rootfs_addon_file"
    exit 1
}

# RD can rename the filename of debian rootfs so as not to include/update debian rootfs for installing
[ -r "$debian_rootfs_file" ] || {
    echo "Warning: Unable to read Debian rootfs file: $debian_rootfs_file"
#    exit 1
}


tmp_dir=
clean_up()
{
    rm -rf $tmp_dir
    exit $1
}

# make the data archive
# contents:
#   - install.sh
#   - grub.cfg
#   - kernel image
#   - aos archive file
#   - debian rootfs addon archive file
#   - debian rootfs(optional)
#   - machine.conf
#   - cicada linux rootfs(i.e. customized ONL rootfs)
#   - cicada linux kernel
#   - cicada initramfs

echo -n "Building self-extracting install image ."
tmp_dir=$(mktemp -d)
tmp_installdir="$tmp_dir/installer"
mkdir $tmp_installdir || clean_up 1

cp $installer_dir/install.sh $tmp_installdir || clean_up 1
chmod 755 $tmp_installdir/install.sh
echo -n "."

cp $installer_dir/grub.cfg $tmp_installdir || clean_up 1
echo -n "."

cp $kernel_file $tmp_installdir/ || clean_up 1
echo -n "."

cp $initrd_file $tmp_installdir/ || clean_up 1
echo -n "."

cp $aos_file $tmp_installdir/ || clean_up 1
echo -n "."

cp $debian_rootfs_addon_file $tmp_installdir/ || clean_up 1
echo -n "."

cp $debian_rootfs_file $tmp_installdir/ || clean_up 1
echo -n "."

echo "support_machines=\"${support_machines}\"" > $tmp_installdir/machine.conf
echo -n "."

cp $ACCPROJ/runtime/cicada_aos5810_54x_rootfs.tar.xz $tmp_installdir/ || clean_up 1
echo -n "."

cp $ACCPROJ/runtime/kernel-3.16-lts-x86_64-all $tmp_installdir/ || clean_up 1
echo -n "."

cp $ACCPROJ/runtime/x86-64-accton-as5812-54x-r0.cpio.gz $tmp_installdir/ || clean_up 1
echo -n "."

sharch="$tmp_dir/sharch.tar"
tar -C $tmp_dir -cf $sharch installer || {
    echo "Error: Problems creating $sharch archive"
    clean_up 1
}
echo -n "."

[ -f "$sharch" ] || {
    echo "Error: $sharch not found"
    clean_up 1
}
sha1=$(cat $sharch | sha1sum | awk '{print $1}')
echo -n "."
cp $common_dir/sharch_body.sh $output_file || {
    echo "Error: Problems copying sharch_body.sh"
    clean_up 1
}

# Need set write permission to $output_file because
# $common_dir/sharch_body.sh is read-only file.
chmod u+w $output_file

# Replace variables in the sharch template
sed -i -e "s/%%IMAGE_SHA1%%/$sha1/" $output_file
sed -i -e "s/%%AOS_RUNTIME_VER%%/$aos_runtime_image_ver/" $output_file
sed -i -e "s/%%AOS_INSTALLER_FILENAME%%/$output_file/" $output_file
sed -i -e "s/%%AOS_INSTALLER_SCRIPT_VER%%/$AOS_INSTALLER_SCRIPT_VER/" $output_file
echo -n "."

# Calculate the header checksum and replace the variable in the sharch template
calc_header_sha1=$(sed '1,/header_sha1=%%IMAGE_HEADER_SHA1%%$/d' $output_file | sha1sum | awk '{ print $1 }')
sed -i -e "s/%%IMAGE_HEADER_SHA1%%/$calc_header_sha1/" $output_file
echo -n "."

cat $sharch >> $output_file
#rm -rf $tmp_dir
echo " Done."

echo "ONIE installer images are generated:"
ls -l | grep ${output_file}

clean_up 0
