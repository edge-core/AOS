#!/bin/sh

# Check argument number
if [ $# -lt 1 ]; then
        echo "Usage: $0 <installer filename in '$AOS_FS_TOP_DIR'>"
        exit 1
fi

installer_file=$1

if [ $# -eq 2 ]; then
    root=$2
else
    root=/
fi

AOS_FS_TOP_DIR=${root}/flash/.fs

export aos_skip_copy_installer=1
export aos_installer_filename=${installer_file}
export aos_root=${root}
$AOS_FS_TOP_DIR/${installer_file} $AOS_FS_TOP_DIR/${installer_file}

