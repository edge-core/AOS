#!/bin/sh

ON_BOARD_USB_TOP_DIR=/mnt/p1
ON_BOARD_USB_CICADA_TOP_DIR=/mnt/p3
ON_BOARD_USB_ORG_MOUNT_STATUS=1
BOOT_TOP_DIR=/mnt/p2
VG_NAME=ACCTON
linux_filesystem_guid="0FC63DAF-8483-4772-8E79-3D69D8477DE4"
aos_boot_label="AOS-BOOT"
aos_boot_uuid="19724fa0-e628-45ba-a277-933b3c6c1d03"
aos_on_board_label="SYSROOT1"          #AOS-SYSROOT"
aos_on_board_uuid="4ee66398-5b8b-4f9a-b12a-8ab49239848a"
aos_on_board_lv="sysroot1"
cicada_on_board_label="SYSROOT_CICADA"
cicada_on_board_uuid="4ee66398-5b8b-4f9a-b12a-8ab49239848b"
cicada_on_board_lv="sysroot_cicada"
cicada_config_label="ONL-CONFIG"
cicada_config_lv="config_cicada"
AOS_BOOT_DEV=
DIAG_PART=
FS_SAVE_FOLDER="/flash/.fs"
LICENSE_FILE=""
aos_machine=

INSTALLER_UID=278
INSTALLER_STARTUP_UID=$(($INSTALLER_UID+1))

INVALID_BOARD_ID=255
board_id=${INVALID_BOARD_ID}

CREATE_AOS_PERSIST=1

##############################
#
# Find the main GPT device in $DEV,
# and the starting switch light partition in $MINPART
#
##############################

CR="
"

# Running under ONIE, most likely in the background in installer mode.
# Our messages have to be sent to the console directly, not to stdout.
installer_say()
{
  echo "$@" > /dev/console
}

# Check the mount point of the first partition of the on-board usb disk.
# This function will update $ON_BOARD_USB_PART1_TOP_DIR if the partition is
# already mounted when calling this function
check_on_board_usb_mount()
{
    local tmp

    tmp="`mount | grep $aos_on_board_uuid`"
    test ! "$tmp" && tmp="`mount | grep "/dev/mapper/$VG_NAME-$aos_on_board_lv"`"
    if test "$tmp"; then
        ON_BOARD_USB_ORG_MOUNT_STATUS=0
        ON_BOARD_USB_TOP_DIR=$aos_root
    fi

    return 0
}

find_aos_boot_dev()
{
    local line

    line="`blkid | grep "$aos_boot_label"`"
    if test "$line"; then
        AOS_BOOT_DEV=`echo "$line" | awk '{print $1}' | sed -e 's/://g'`
    else
        installer_say "Get AOS-BOOT DEV fail"
    fi

    return 0
}

visit_blkid()
{
  local fn rest
  fn=$1; shift
  rest="$@"

  local ifs
  ifs=$IFS; IFS=$CR
  for line in `blkid`; do
    IFS=$ifs

    local part rest
    part="`echo "$line" | sed -e 's/:.*//'`"
    rest="`echo "$line" | sed -e 's/^[^:]*:[ ]*//'`"

    local LABEL UUID key val
    while test "$rest"; do
      key="`echo "$rest" | sed -e 's/=.*//'`"
      val="`echo "$rest" | sed -e 's/^[^=]*=\"\([^\"]*\).*\"/\1/'`"
      rest="`echo "$rest" | sed -e 's/^[^=]*=\"[^\"]*\"[ ]*//'`"
      eval "$key=\"$val\""
    done

    eval $fn "$part" "$LABEL" "$UUID" $rest || return 1

    # init LABEL UUID
    LABEL=
    UUID=
  done
  IFS=$ifs

  return 0
}

DEV=
MINPART=

update_minpart()
{
  local dev part
  dev=$1; shift
  part=$1; shift

  DEV=$dev
  if test "$MINPART"; then
    if test $MINPART -le $part; then
      MINPART=$(( $part + 1 ))
    fi
  else
    MINPART=$(( $part + 1 ))
  fi  
}

do_visit_blkid_part()
{
  local dev part label uuid
  part=$1; shift
  label=$1; shift
  uuid=$1; shift

  case "$part" in
    /dev/???[0-9])
      dev="`echo $part | sed -e 's/[0-9]$//'`"
      part="`echo $part | sed -e 's/.*\([0-9]\)$/\1/'`"
    ;;
    *)
      installer_say "*** invalid partition $part"
      return 1
      ;;
  esac

  case "$label" in
    AOS-PERSIST)
      CREATE_AOS_PERSIST=0
      update_minpart $dev $part || return 1
    ;;

    ONIE-BOOT|*-PERSIST)
      update_minpart $dev $part || return 1
    ;;

    *-DIAG)
      DIAG_PART=$part
    ;;
  esac
}

find_gpt()
{
  DEV=
  MINPART=
  visit_blkid do_visit_blkid_part || return 1
  if test -b "$DEV"; then
    :
  else
    installer_say "*** cannot find install device"
    return 1
  fi
  if test "$MINPART"; then
    if [ $MINPART -eq $DIAG_PART ]; then
        update_minpart $DEV $DIAG_PART || return 1
    fi
  else
    installer_say "*** cannot find install partition"
    return 1
  fi
  return 0
}

visit_parted()
{
  local dev diskfn partfn rest
  dev=$1; shift
  diskfn=$1; shift
  partfn=$1; shift
  rmfn=$1; shift
  rest="$@"

  local ifs ifs2 dummy
  ifs=$IFS; IFS=$CR
  for line in `parted -m $dev unit s print`; do
    IFS=$ifs

    line=`echo "$line" | sed -e 's/[;]$//'`

    case "$line" in
      /dev/*)
        ifs2=$IFS; IFS=:
        set dummy $line
        IFS=$ifs2

        local dev sz model lbsz pbsz typ modelname flags
        shift
        dev=$1; shift
        sz=$1; shift
        model=$1; shift
        lbsz=$1; shift
        pbsz=$1; shift
        typ=$1; shift
        modelname=$1; shift
        flags=$1; shift

        eval $diskfn "$dev" "$sz" "$model" "$typ" "$flags" $rest || return 1

        ;;
      [0-9]:*)
        ifs2=$IFS; IFS=:
        set dummy $line
        IFS=$ifs2

        local part start end sz fs label flags
        shift
        part=$1; shift
        start=$1; shift
        end=$1; shift
        sz=$1; shift
        fs=$1; shift
        label=$1; shift
        flags=$1; shift

        eval $partfn "$rmfn" "$part" "$start" "$end" "$sz" "$fs" "$label" "$flags" $rest || return 1

        ;;

      *) continue ;;
    esac

  done
  IFS=$ifs
}

BLOCKS=
# total blocks on this GPT device

NEXTBLOCK=
# next available block for allocating partitions

do_handle_disk()
{
  local dev sz model typ flags
  dev=$1; shift
  sz=$1; shift
  model=$1; shift
  typ=$1; shift
  flags=$1; shift

  if test "$typ" != "gpt"; then
    installer_say "*** invalid partition table: $typ"
    return 1
  fi
  BLOCKS=`echo "$sz" | sed -e 's/[s]$//'`
  installer_say "found a disk with $BLOCKS blocks"

  return 0
}

do_maybe_delete()
{
  local part start end sz fs label flags
  rmfn=$1; shift
  part=$1; shift
  start=$1; shift
  end=$1; shift
  sz=$1; shift
  fs=$1; shift
  label=$1; shift
  flags=$1; shift

  installer_say "examining $DEV part $part"
  if test $part -lt $MINPART; then
    echo "skip this part"
    end=`echo "$end" | sed -e 's/[s]$//'`
    if test "$NEXTBLOCK"; then
      if test $end -ge $NEXTBLOCK; then
        NEXTBLOCK=$(( $end + 1 ))
      fi
    else
      NEXTBLOCK=$(( $end + 1 ))
    fi
    return 0
  fi

  eval $rmfn "$DEV" "$part" || return 1
  return 0
}

do_remove_lvm()
{
  local dev line label
  
  dev=$1$2
      
  line="`pvdisplay $dev 2>&1 | grep "VG Name"`"
        
  if test "$line"; then
    local vg
              
    vg=`echo "$line" | awk '{print $3}'`

    installer_say "deleting this lvm"
                  
    if test "$vg"; then
      lvremove -f $vg || return 1
      vgremove -f $vg || return 1
    fi
    
    pvremove -f $dev || return 1
  fi
                                              
  return 0
}

do_remove_part()
{
  installer_say "deleting this part"

  parted $1 rm $2 || return 1
  
  return 0
}

# final partitions

partition_gpt()
{
  local start end part vg

  if [ $CREATE_AOS_PERSIST -eq 1 ]; then
    installer_say "Create 50M for AOS-PERSIST"

    start=$NEXTBLOCK
    end=$(( $start + 97617))

    parted -s $DEV unit s mkpart AOS-PERSIST ext4 ${start}s ${end}s || return 1
    mkfs.ext4 -L AOS-PERSIST ${DEV}${MINPART}
    sgdisk -t $MINPART:$linux_filesystem_guid $DEV

    NEXTBLOCK=$(( $end + 1 ))
    MINPART=$(( $MINPART + 1 ))
  fi

  installer_say "Creating 210M for AOS boot"
  start=$NEXTBLOCK
  end=$(( $start + 409599 ))

  parted -s $DEV unit s mkpart $aos_boot_label ext4 ${start}s ${end}s || return 1
  parted $DEV set $MINPART boot on || return 1
  mkfs.ext4 -L $aos_boot_label -U $aos_boot_uuid ${DEV}${MINPART}
  # in order to let the aos-boot partition be removed by ONIE command "onie-uninstall"
  # the guid of the aos-boot partition will be set as the guid for usual linux
  # filesystem data
  sgdisk -t $MINPART:$linux_filesystem_guid $DEV
  AOS_BOOT_DEV=${DEV}${MINPART}

  NEXTBLOCK=$(( $end + 1 ))
  MINPART=$(( $MINPART + 1 ))

  installer_say "Allocating remainder for LVM"
  start=$NEXTBLOCK

  parted -s $DEV unit s mkpart aos_lvm ext4 ${start}s "100%" || return 1
  parted -s $DEV set ${MINPART} lvm on
  vg=${DEV}${MINPART}

  pvcreate $vg
  vgcreate $VG_NAME $vg
  lvcreate -L 2G -n $aos_on_board_lv /dev/$VG_NAME
  lvcreate -L 1G -n $cicada_on_board_lv /dev/$VG_NAME
  lvcreate -L 4M -n $cicada_config_lv /dev/$VG_NAME

  mkfs.ext4 -L $aos_on_board_label -U $aos_on_board_uuid /dev/$VG_NAME/$aos_on_board_lv
  mkfs.ext4 -L $cicada_on_board_label -U $cicada_on_board_uuid /dev/$VG_NAME/$cicada_on_board_lv
  mkfs.ext4 -L $cicada_config_label /dev/$VG_NAME/$cicada_config_lv

  return 0
}

umount_on_board_usb_disk()
{
  umount $BOOT_TOP_DIR
  rm -r $BOOT_TOP_DIR

if [ $ON_BOARD_USB_ORG_MOUNT_STATUS -eq 1 ]; then
  umount $ON_BOARD_USB_TOP_DIR
  rm -r $ON_BOARD_USB_TOP_DIR
fi

  umount $ON_BOARD_USB_CICADA_TOP_DIR
  rm -r $ON_BOARD_USB_CICADA_TOP_DIR
}

# main entry point

# check required environment variable
if [ -z "$aos_installer_filename" ] ; then
    echo "Error! \$aos_installer_filename is not defined."
    exit 1
fi

# check number of argument
if [ $# -lt 1 ]; then
    echo "Error! Incorrect number of argument"
    exit 1
fi

installer_file_path=$1

check_on_board_usb_mount

cd $(dirname $0)

if [ -n "$aos_do_validation_only" ] || [ $ON_BOARD_USB_ORG_MOUNT_STATUS -eq 1 ]; then
    cat machine.conf
    . ./machine.conf

    # Pickup ONIE defines for this machine.
    [ -r /etc/machine.conf ] && . /etc/machine.conf

    echo "Installer: support_machines: $support_machines"
    #echo "Dumping Install Environment:[Start]"
    #export
    #set
    #echo "Dumping Install Environment:[End]"

    if  test ! "$onie_machine"; then
        onie_machine="`onlp_query -ep | sed -e 's/-/_/g'`"
        echo "$onie_machine"
    fi

    #support_machines is defined in ./machine.conf
    match_found=1
    for machine in $support_machines
    do
        echo $onie_machine | egrep ".*$machine.*"
        if [ $? = "0" ]; then
            echo "Match found!($machine)"
            match_found=0
            aos_machine=$machine
        fi
    done

    if [ $match_found = "1" ]; then
        echo "Error! The installer is not for this device. Installation abort."
        exit 1
    fi

    if [ -n "$aos_do_validation_only" ]; then
        echo "Image validation OK."
        exit 0
    fi
fi

if [ $ON_BOARD_USB_ORG_MOUNT_STATUS -eq 1 ]; then
    find_gpt
    installer_say "Installing to $DEV starting at partition $MINPART"

    visit_parted $DEV do_handle_disk do_maybe_delete do_remove_lvm 
    visit_parted $DEV do_handle_disk do_maybe_delete do_remove_part
    partition_gpt

    # mount for AOS-BOOT
    if [ ! -d $BOOT_TOP_DIR ]; then
        mkdir -p $BOOT_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "Error! Failed to create directory $BOOT_TOP_DIR."
            exit 1
        fi
    fi

    mount -t ext4 $AOS_BOOT_DEV $BOOT_TOP_DIR
    if [ $? != "0" ]; then
        installer_say "Error! Failed to mount the directory $BOOT_TOP_DIR."
        exit 1
    fi

    # install for initrd
    if [ ! -f initrd.cpio.gz ];  then
        installer_say "Info: initrd image is not included for installation."
        exit 1
    else
        cp -f initrd.cpio.gz $BOOT_TOP_DIR
        cp -f x86-64-accton-as5812-54x-r0.cpio.gz $BOOT_TOP_DIR
    fi

    # mount for SYSROOT
    #for Debian rootfs
    if [ ! -d $ON_BOARD_USB_TOP_DIR ]; then
        mkdir -p $ON_BOARD_USB_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "Error! Failed to create directory $ON_BOARD_USB_TOP_DIR."
            exit 1
        fi
    fi

    mount -t ext4 UUID=$aos_on_board_uuid $ON_BOARD_USB_TOP_DIR
    if [ $? != "0" ]; then
        installer_say "Error! Failed to mount the directory $ON_BOARD_USB_TOP_DIR."
        exit 1
    fi

    if [ ! -f onl_rootfs.tar.xz ];  then
        installer_say "Info: Debian rootfs file is not included for installation."
    else
        # backup the license file
        if [ -f $ON_BOARD_USB_TOP_DIR$LICENSE_FILE ]; then
        cp -pf $ON_BOARD_USB_TOP_DIR$LICENSE_FILE /tmp/license.lic
        fi

        installer_say "Removing existing rootfs..."
        cd $ON_BOARD_USB_TOP_DIR
        rm -rf * .??* .[!.]*
        cd -

        installer_say "Decompressing  rootfs..."
        unxz onl_rootfs.tar.xz
        tar xf onl_rootfs.tar -C $ON_BOARD_USB_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "----------------------------------------------"
            installer_say "Failed to decompress onl_rootfs.tar.xz !!!"
            installer_say "----------------------------------------------"
            exit 1
        fi
        rm onl_rootfs.tar
    fi # end of if [ ! -f onl_rootfs.tar.gz ]

    mkdir -p $ON_BOARD_USB_TOP_DIR/tmp/lock
    mkdir -p $ON_BOARD_USB_TOP_DIR/tmp/run
    mkdir -p $ON_BOARD_USB_TOP_DIR/tmp/log
 
    # restore the license file
    if [ -f /tmp/license.lic ]; then
        mv -f /tmp/license.lic $ON_BOARD_USB_TOP_DIR$LICENSE_FILE
    fi

    # because UIs upgrade installer using /tmp to compute checksum, need /tmp to be big enough
    sed -i '/\/tmp/d' $ON_BOARD_USB_TOP_DIR/etc/fstab
    echo 'tmpfs /tmp     tmpfs rw,noatime,size=256M,mode=1777   0 0' >> $ON_BOARD_USB_TOP_DIR/etc/fstab

    # create ONL file
    touch $ON_BOARD_USB_TOP_DIR/etc/onl_block
    echo $onie_platform | sed -r 's/_/-/g'>$ON_BOARD_USB_TOP_DIR/etc/onl_platform
    echo 'Open Network Linux  373b016 (amd64.all,2016.03.31.04.14,373b01641c1409836b007259ce3d9267ac34b284)' >$ON_BOARD_USB_TOP_DIR/etc/onl_version
    echo "pci0000:00/0000:00:14.0 CRAFT" >$ON_BOARD_USB_TOP_DIR/etc/onl_net

    # remove snmp ssh lldp loadstartupconfig 
    # must execute update-rc.d after remove
    rm $ON_BOARD_USB_TOP_DIR/etc/init.d/snmpd
    rm $ON_BOARD_USB_TOP_DIR/etc/init.d/ssh
    rm $ON_BOARD_USB_TOP_DIR/etc/init.d/lldpd
    rm $ON_BOARD_USB_TOP_DIR/etc/init.d/loadstartupconfig

    # sed -i '$ s/$/\nauto CRAFT \niface CRAFT inet dhcp/g' $ON_BOARD_USB_TOP_DIR/etc/network/interfaces

    # mount for CICADA SYSROOT
    if [ ! -d $ON_BOARD_USB_CICADA_TOP_DIR ]; then
        mkdir -p $ON_BOARD_USB_CICADA_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "Error! Failed to create directory $ON_BOARD_USB_CICADA_TOP_DIR."
            exit 1
        fi
    fi
    mount -t ext4 UUID=$cicada_on_board_uuid $ON_BOARD_USB_CICADA_TOP_DIR
    if [ $? != "0" ]; then
        installer_say "Error! Failed to mount the directory $ON_BOARD_USB_CICADA_TOP_DIR."
        exit 1
    fi

    if [ ! -f cicada_aos5810_54x_rootfs.tar.xz ];  then
        installer_say "Info: Cicada AOS5810-54X rootfs file is not included for installation."
    else
        installer_say "Removing existing rootfs..."
        cd $ON_BOARD_USB_CICADA_TOP_DIR
        rm -rf * .??* .[!.]*
        cd -

        installer_say "Decompressing Cicada AOS5810-54X rootfs..."
        unxz cicada_aos5810_54x_rootfs.tar.xz
        tar xf cicada_aos5810_54x_rootfs.tar -C $ON_BOARD_USB_CICADA_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "----------------------------------------------"
            installer_say "Failed to decompress cicada_aos5810_54x_rootfs.tar.xz !!!"
            installer_say "----------------------------------------------"
            exit 1
        fi
        rm cicada_aos5810_54x_rootfs.tar
    fi # end of if [ ! -f cicada_aos5810_54x_rootfs.tar.xz ];  then

    #install GRUB for AOS
    mkdir -p $BOOT_TOP_DIR/grub
    cp grub.cfg "$BOOT_TOP_DIR/grub/grub.cfg"
    #append onl_platform variable for Cicada Linux grub cfg
    onl_platform=$(echo $onie_platform | sed -r 's/_/-/g')
    sed -i "s/^[ \t]*linux[ \t]*\/kernel-3.16-lts-x86_64-all.*$/& onl_platform=$onl_platform/" $BOOT_TOP_DIR/grub/grub.cfg

    installer_say "Installing GRUB"
    grub-install --boot-directory=$BOOT_TOP_DIR $DEV

else
    # mount for AOS-BOOT
    find_aos_boot_dev
    if [ ! -d $BOOT_TOP_DIR ]; then
        mkdir -p $BOOT_TOP_DIR
        if [ $? != "0" ]; then
            installer_say "Error! Failed to create directory $BOOT_TOP_DIR."
            exit 1
        fi
    fi

    mount -t ext4 $AOS_BOOT_DEV $BOOT_TOP_DIR
    if [ $? != "0" ]; then
        installer_say "Error! Failed to mount the directory $BOOT_TOP_DIR."
        exit 1
    fi

    # rm old AOS_FILE
    sed -i 's/\.\//\'$(echo $ON_BOARD_USB_TOP_DIR)'\//g' $ON_BOARD_USB_TOP_DIR/usr/share/aos/Manifest
    xargs rm -r < $ON_BOARD_USB_TOP_DIR/usr/share/aos/Manifest
fi #end of if [ $ON_BOARD_USB_PART1_ORG_MOUNT_STATUS -eq 1 ]

#install kernel
installer_say "Install KERNEL image..."    
if [ ! -f aos_vmlinuz ];  then
    installer_say "Info: kernel image is not included for installation."
    exit 1
else
    cp -f aos_vmlinuz $BOOT_TOP_DIR
    cp -f kernel-3.16-lts-x86_64-all $BOOT_TOP_DIR
fi

installer_say "Install AOS file..."
if [ -f onl_rootfs_addon.tar.xz ]; then
    unxz onl_rootfs_addon.tar.xz
    tar xf onl_rootfs_addon.tar --overwrite -C $ON_BOARD_USB_TOP_DIR
    if [ $? != "0" ]; then
        installer_say "----------------------------------------------"
        installer_say "Failed to decompress onl_rootfs_addon.tar.xz !!!"
        installer_say "----------------------------------------------"
        exit 1
    fi
    rm onl_rootfs_addon.tar
fi

tar zxf aos_files.tgz -C $ON_BOARD_USB_TOP_DIR/
if [ $? != "0" ]; then
    installer_say "----------------------------------------------"
    installer_say "Failed to install aos_file.tgz !!!"
    installer_say "----------------------------------------------"
    exit 1
fi

# link for onlp
if [ -n "$aos_machine" ]; then
    ln -s -f onlp_platmgrd_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/sbin/onlp_platmgrd
    ln -s -f onlp_fanutil_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_fanutil
    ln -s -f onlp_psuutil_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_psuutil
    ln -s -f onlp_query_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_query
    ln -s -f onlp_sfputil_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_sfputil
    ln -s -f onlp_thermalutil_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_thermalutil
    ln -s -f onlp_ledutil_$aos_machine $ON_BOARD_USB_TOP_DIR/usr/bin/onlp_ledutil
fi

# update rc.d for onl_rootfs
# do this after install AOS file
line="`cat $ON_BOARD_USB_TOP_DIR/etc/init.d/sendsigs | grep "Required-Stop"`"
if [ -n "$line" ]; then
    aos="`echo $line | grep "aos"`"
    if test ! "$aos"; then
        sed -i "s/Required-Stop:/Required-Stop:    aos/g" $ON_BOARD_USB_TOP_DIR/etc/init.d/sendsigs
    fi
fi

chroot $ON_BOARD_USB_TOP_DIR /bin/sh<<\EOF
    update-rc.d aos start 1 3 . stop 1 6 .
EOF

if [ ! -d $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER ]; then
    mkdir -p $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER
    if [ $? != "0" ]; then
        umount_on_board_usb_disk
        echo "Error! Failed to create $FS_SAVE_FOLDER directory."
        exit 1
    fi
fi

# Check whether .postinfo.cfg exists
if [ ! -f $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/.postinfo.cfg ]; then
    # set default post mode as FAST mode.
    echo "4" > $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/.postinfo.cfg
    # set uid of .postinfo.cfg for file type used in FS
    chown 134 $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/.postinfo.cfg
fi

if [ -z "$aos_skip_copy_installer" ]; then
    # Copy the the installer file
    echo "Copying the installer file..."

    # If the same name of $aos_installer_filename already exists on the on-board usb
    # disk, just overwrite the existing file and do not need to remove the
    # non-startup runtime image.
    if [ ! -f $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename ]; then
        # Remove non-startup file if exists.
        non_startup_installer=`find $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/ -user $INSTALLER_UID`
        if [ ! -z $non_startup_installer ]; then
            #echo "Debug:Remove non-startup installer file '$non_startup_installer'."
            rm $non_startup_installer
        fi
    else
        #echo "Debug:Skip the operation of removing non-startup installer file because $aos_installer_filename already exists."
        #ls -l $ON_BOARD_USB_PART1_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename #debug
        rm -f $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename
        if [ $? != "0" ]; then
            echo "Error! Failed to remove the file '$ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename'"
            umount_on_board_usb_disk
            exit 1
        fi
    fi
fi # end of if [ -z "$aos_skip_copy_installer" ]

# Change startup file as no-startup file
startup_installer=`find $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/ -user $INSTALLER_STARTUP_UID`
if [ ! -z $startup_installer ]; then
    #echo "Debug:Set startup installer '$startup_installer' as non-startup."
    chown $INSTALLER_UID $startup_installer
fi

if [ -z "$aos_skip_copy_installer" ]; then
    # Install the new installer image and remove the file mapping file
    # file mapping file will be generated latter in this script
    cp $installer_file_path $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename
    if [ $? != "0" ]; then
        echo "Error! Failed to copy installer image to $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/"
        umount_on_board_usb_disk
        exit 1
    fi
fi

# Change uid to set it as startup installer
chown $INSTALLER_STARTUP_UID $ON_BOARD_USB_TOP_DIR$FS_SAVE_FOLDER/$aos_installer_filename
#echo "Debug:Set '$aos_installer_filename' as startup installer."
if [ $? != "0" ]; then
    echo "Error! Failed to set the installed installer image as startup installer image."
    umount_on_board_usb_disk
    exit 1
fi

echo -e "header_sha1=$header_sha1\npayload_sha1=$payload_sha1" > $ON_BOARD_USB_TOP_DIR/usr/share/aos/CheckSum


umount_on_board_usb_disk
sync; sync


