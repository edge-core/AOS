#!/bin/sh

ACCTON_PLATFORM_MAP=${ACCTON_PLATFORM_TOP_DIR}/project_build_environment/${PROJECT_NAME}/fs/rootfs/usr/map

if [ ! -d ${ACCTON_PLATFORM_MAP} ]; then
    mkdir -p ${ACCTON_PLATFORM_MAP} || ( echo "Failed to create directory ${ACCTON_PLATFORM_MAP}" && exit 1 )
fi

cd ${ACCTON_PLATFORM_TOP_DIR}/project_build_environment/${PROJECT_NAME}/fs/rootfs/usr/bin
FILES=`ls`
for file in $FILES
do
    touch ${ACCTON_PLATFORM_MAP}/${file}.tmpmap
    [ ! -h $file ] && ${CROSS_COMPILE}nm -Dn --defined-only $file | grep -e " T " -e " t " >${ACCTON_PLATFORM_MAP}/${file}.tmpmap 
    if [ -s ${ACCTON_PLATFORM_MAP}/${file}.tmpmap ]; then
        mv -f ${ACCTON_PLATFORM_MAP}/${file}.tmpmap ${ACCTON_PLATFORM_MAP}/${file}.map
    else
        rm ${ACCTON_PLATFORM_MAP}/${file}.tmpmap 2>/dev/null
    fi
done

cd ${ACCTON_PLATFORM_TOP_DIR}/project_build_environment/${PROJECT_NAME}/fs/rootfs/usr/lib
FILES=`ls *.so.0.0.0`
for file in $FILES
do
    touch ${ACCTON_PLATFORM_MAP}/${file}.tmpmap
    ${CROSS_COMPILE}nm -Dn --defined-only $file | grep -e " T " -e " t " >${ACCTON_PLATFORM_MAP}/${file}.tmpmap 
    if [ -s ${ACCTON_PLATFORM_MAP}/${file}.tmpmap ] ; then
        mv -f ${ACCTON_PLATFORM_MAP}/${file}.tmpmap ${ACCTON_PLATFORM_MAP}/${file}.map
    else
        rm ${ACCTON_PLATFORM_MAP}/${file}.tmpmap 2>/dev/null
    fi
done 

# clear all map files that with size 0 to avoid error when dump stack frame
# in linux kernel
cd ${ACCTON_PLATFORM_MAP}
find . -type f -size 0 -exec rm '{}' \;

exit 0
