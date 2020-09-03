#!/bin/sh

machine=$1
installer_dir=$2
common_dir=$3
uimage_file=$4
output_file=$5
uimage_file_name=$6

shift 5

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

[ -r "$uimage_file" ] || {
    echo "Error: Unable to read installer uImage file: $uimage_file"
    exit 1
}

tmp_dir=
clean_up()
{
    rm -rf $tmp_dir
    exit $1
}

# make the data archive
# contents:
#   - uImage file
#   - install.sh

echo -n "Building self-extracting install image ."
tmp_dir=$(mktemp --directory)
tmp_installdir="$tmp_dir/installer"
mkdir $tmp_installdir || clean_up 1

cp $installer_dir/install.sh $tmp_installdir || clean_up 1
echo -n "."

if [ -r $installer_dir/diag_install.sh ] ; then
cp $installer_dir/diag_install.sh $tmp_installdir || clean_up 1
echo -n "."
fi

cp $uimage_file $tmp_installdir/$uimage_file_name || clean_up 1
echo -n "."
echo "machine=$machine" > $tmp_installdir/machine.conf
echo "uimage_file_name=$uimage_file_name" >> $tmp_installdir/machine.conf
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

# Replace variables in the sharch template
sed -i -e "s/%%IMAGE_SHA1%%/$sha1/" $output_file
echo -n "."
cat $sharch >> $output_file
rm -rf $tmp_dir
echo " Done."

echo "Success: install image is ready in ${output_file}:"
ls -l ${output_file}

clean_up 0
