#!/bin/bash

build_onlp=0
debug=0
ONLPLIB_MODEL_NAME_LIST="x86_64_accton_as5812_54x x86_64_accton_as5812_54t x86_64_accton_as6812_32x"
ONLPLIB_BUILD_ARCH_NAME=x86_64-linux-gnu
DOCKER_IMAGE_PREFIX=opennetworklinux/onl-build
DOCKER_IMAGE_VER=1.7
BUILD_ONLP_AUTO=y

if ! ./check_build_env; then
    . ./build_env_init
fi

ONLPLIB_DOCKERFILE_DIR=${ACCPROJ}/user/driver/devdrv/onlp/onlp-accton
ONLP_VERSION=onlp_v1
DOCKER_SHELL=${ONLPLIB_DOCKERFILE_DIR}/build_onlplib_docker_shell.sh

# parse arguments
while getopts ":bgh" opt; do
    case $opt in
        b)
            build_onlp=1
            ;;
        g)
            debug=1
            ;;
        h)
            cat << EOF
Usage: $0 [-bgh]
       -b  Build onlp library
       -g  Enable debug mode
       -h  Show help message
       If no option is specified, it will extract onlp
       source code files if it has not been extracted yet.
EOF
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            echo "Use -h to show usage"
            exit 0
            ;;
    esac
done

ONLPLIB_SRC_DIR=${ACCSRC}/user/driver/devdrv/onlp/${ONLP_VERSION}

if [ ! -d ${ONLPLIB_SRC_DIR} ] ; then
    echo "Extracting ${ONLP_VERSION}_git.tgz ..."
    cd ${ONLPLIB_SRC_DIR%%${ONLP_VERSION}} && tar zxf ${ONLP_VERSION}_git.tgz

    if [ $? -ne 0 ]; then
        echo "Failed to extract onlp-accton_${MODEL_NAME}_git.tgz"
        exit 1
    fi
else
    echo " ${ONLPLIB_SRC_DIR} already exists. Skip extracting file."
fi


if [ ${build_onlp} -eq 1 ]; then
    echo "Build ONLP"

# Check whether docker is available
    if [ ! $(command -v docker) ]; then
        echo "Error:docker is not available."
        exit 1
    fi

# Check whether the docker image is availabe
    docker images | grep -e "${DOCKER_IMAGE_PREFIX}[ \t]*${DOCKER_IMAGE_VER}" > /dev/null
    if [ ! $? ]; then
        cd ${ONLPLIB_DOCKERFILE_DIR} && docker build -t ${DOCKER_IMAGE_PREFIX}:${DOCKER_IMAGE_VER} .
        if [ $? -ne 0 ]; then
            echo "Error: build docker image failed."
            exit 1
        fi
    fi

    if [ $debug -eq 1 ]; then
        DOCKER_INTERACTIVE=-i
        BUILD_ONLP_AUTO=n
    else
        DOCKER_INTERACTIVE=
    fi

    echo "docker run"
    for ONLPLIB_MODEL_NAME in $ONLPLIB_MODEL_NAME_LIST
    do
        docker run --privileged -t --rm\
            ${DOCKER_INTERACTIVE} \
            -e USER=root \
            -e MAKE_UID=`id -u` \
            -e MAKE_USER=${USER} \
            -e ONLPLIB_MODEL_NAME=${ONLPLIB_MODEL_NAME} \
            -e BUILD_ONLP_AUTO=${BUILD_ONLP_AUTO} \
            -v ${ONLPLIB_SRC_DIR}:/home/onlp-accton \
            -v `cd ${ACCSRC}/../ && pwd`:`cd ${ACCSRC}/../&& pwd` \
            -w /home/onlp-accton \
            -h "onl-builder" \
            ${DOCKER_IMAGE_PREFIX}:${DOCKER_IMAGE_VER} \
            bash -c ${DOCKER_SHELL}

        if [ $? -ne 0 ]; then
            echo "Make ONLP error."
            exit 1
        fi

        OUTPUT_LIB_DIR=${ONLPLIB_SRC_DIR}/targets/${ONLPLIB_MODEL_NAME}/onlpdump/build/${ONLPLIB_BUILD_ARCH_NAME}/lib
        cp -f ${OUTPUT_LIB_DIR}/*.a ${ACCPROJ}/user/driver/devdrv/onlp/onlp-accton
        if [ $? -ne 0 ]; then
            echo "Copy ONLP libraries error."
            exit 1
        fi
    done
    echo "Copy ONLP libraries done."
fi


echo "build_onlplib.sh Done."
exit 0
