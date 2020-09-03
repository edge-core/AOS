#!/bin/bash

DBG_FLAG=0

function create_dir () {
    if [ $DBG_FLAG == 1 ]; then
        sleep 2
        return 1
    fi

    test -d ${ACCPROJ}/prebuilt && rm -rf ${ACCPROJ}/prebuilt
    mkdir ${ACCPROJ}/prebuilt/
    pushd ${ACCPROJ}/prebuilt/ > /dev/null
    mkdir sdk driver process
}

function copy_files () {
    if [ $DBG_FLAG == 1 ]; then
        sleep 2
        return 1
    fi

    cp ${SDKPROJ}/build/*.ko sdk/
    cp ${SDKPROJ}/build/unix-user/aos7710_32x/libbcmsdk.a sdk/

    cp -rf ${ACCPROJ}/user/driver/devdrv driver/
    cp -rf ${ACCPROJ}/user/driver/swdrv  driver/
    cp -rf ${ACCPROJ}/user/process/driver_proc process/
    cp ${ACCPROJ}/githash.txt ./version.txt
}

function del_files () {
    if [ $DBG_FLAG == 1 ]; then
        sleep 2
        return 1
    fi

    rm -rf driver/devdrv/onlp

    find . -type f ! \( -name '*.so*' -or -name '*.a' -or -name '*.la' -or -name 'driver_proc' -or -name '*.ko' -or -name 'version.txt' \) -delete

    find . -type d \( -name '*.deps' \) | xargs rm -rf

    rm process/driver_proc/driver_proc
}

function strip_syms () {
    if [ $DBG_FLAG == 1 ]; then
        sleep 2
        return 1
    fi

    find . ! -type d  \( -name '*.so' -or -name '*_proc' \) | xargs ${CROSS_COMPILE}strip

    find . -name '*.a' | xargs ${CROSS_COMPILE}strip -d
}

function make_tgz () {
    if [ $DBG_FLAG == 1 ]; then
        sleep 2
        return 1
    fi

    tar czvf ../prebuilt.tgz .

    echo -e '\n'
    ls -al ../prebuilt.tgz
}

fun_arr=( create_dir copy_files del_files strip_syms make_tgz )
hed_arr=( "### create directorys ###" \
          "### copy needed files ###" \
          "### delete unneeded files ###" \
          "### strip symbols ###" \
          "### generate prebuilt.tgz ###" )

for idx in ${!fun_arr[*]}; do
    printf "\n%s.%s\n" $((idx+1)) "${hed_arr[$idx]}"

    ${fun_arr[$idx]} &

    pid_f="$!"

    while kill -0 "$pid_f" 2> /dev/null ; do
        printf '.'
    sleep 1
    done
    echo ' done'

    if [ $idx == 0 ]; then
        pushd ${ACCPROJ}/prebuilt/ > /dev/null
    fi
done
