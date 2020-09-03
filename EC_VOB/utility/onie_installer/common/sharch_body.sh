#!/bin/sh
header_sha1=%%IMAGE_HEADER_SHA1%%
## header_sha1 must be put in the 2nd line!!!

##
## Shell archive template
##
## Strings of the form %%VAR%% are replaced during construction.
##

## variables that need to be exported before calling install.sh
aos_runtime_ver=%%AOS_RUNTIME_VER%%
if [ -z "$aos_installer_filename" ]; then
    aos_installer_filename=%%AOS_INSTALLER_FILENAME%%
fi
aos_installer_script_ver=%%AOS_INSTALLER_SCRIPT_VER%%

echo -n "Verifying image header checksum ..."
sha1=$(sed -e '1,/^exit_marker$/d' "$0" | sha1sum | sed 's/\(^\w\+\)\b.*$/\1/')
calc_header_sha1=$(sed -n -e '1,/^exit_marker$/p' "$0" | sed -e '1,/header_sha1=.*$/d' | sha1sum | sed 's/\(^\w\+\)\b.*$/\1/')

payload_sha1=%%IMAGE_SHA1%%

if [ "$calc_header_sha1" != "$header_sha1" ] ; then
    echo
    echo "Error! Archive header checksum error"
    echo "Expected: $header_sha1"
    echo "Found   : $calc_header_sha1"
    exit 1
fi
echo " OK."

echo -n "Verifying image payload checksum ..."
if [ "$sha1" != "$payload_sha1" ] ; then
    echo
    echo "Error! Image payload checksum error"
    echo "Expected: $payload_sha1"
    echo "Found   : $sha1"
    exit 1
fi
echo " OK."

# Untar and launch install script in a tmpfs
cur_wd=$(pwd)
#archive_path=$(realpath "$0")
#archive_path=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)/`basename "${BASH_SOURCE[0]}"`
archive_path=$(cd `dirname "$0"` && pwd)/`basename "$0"`

tmp_dir=$(mktemp -dt) || {
        echo "Error! mktemp returns error"
        exit 1
}

if [ "$(id -u)" = "0" ] ; then
    #echo "Debug:mount -t tmpfs tmpfs-installer $tmp_dir"
    mount -t tmpfs tmpfs-installer $tmp_dir || {
        echo "Error! Failed to mount tmpfs upon '$tmp_dir'"
        exit 1
    }
else
	true # dummy line to avoid error
    #echo "Debug: not mount tmpfs"
fi
cd $tmp_dir
echo -n "Preparing image archive ..."
sed -e '1,/^exit_marker$/d' $archive_path | tar xf - || exit 1
echo " OK."
cd $cur_wd
if [ -n "$extract" ] ; then
    # stop here
    echo "Image extracted to: $tmp_dir"
    if [ "$(id -u)" = "0" ] && [ ! -d "$extract" ] ; then
        echo "To un-mount the tmpfs when finished type:  umount $tmp_dir"
    fi
    exit 0
fi

#export required variables before calling install.sh
export aos_installer_filename
export header_sha1
export payload_sha1
[ -n "$aos_do_validation_only" ] && export aos_do_validation_only
$tmp_dir/installer/install.sh $0
rc="$?"

# clean up
if [ "$(id -u)" = "0" ] ; then
    umount $tmp_dir
fi
rm -rf $tmp_dir

exit $rc
exit_marker
