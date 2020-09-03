#!/bin/sh

# This is the script to be executed in docker for building ONLP library

# Start the local apt caching daemon
/etc/init.d/apt-cacher-ng start
echo "build start"
# Create an account with the calling UID and add it to sudo
if [ ! -z $MAKE_USER ] ; then
    useradd $MAKE_USER --uid $MAKE_UID -m
    echo "$MAKE_USER     ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
    chown $MAKE_USER:$MAKE_USER /home/$MAKE_USER /root       # make sure we can write to home dir
    if [ "x${BUILD_ONLP_AUTO}" != "xy" ] ; then
        sudo -u $MAKE_USER ONL=$ONL bash
    fi
else
    if [ "x${BUILD_ONLP_AUTO}" != "xy" ] ; then
        bash
    fi
fi

if [ "x${BUILD_ONLP_AUTO}" = "xy" ] ; then
    echo "build ${ONLPLIB_MODEL_NAME}"
    echo "#!/bin/bash" > /tmp/build.sh
    echo "cd /home/onlp-accton/targets/${ONLPLIB_MODEL_NAME} && make all" >> /tmp/build.sh
    chmod +x /tmp/build.sh
    sudo -u $MAKE_USER /tmp/build.sh
    if [ $? -ne 0 ]; then
        echo "Build ONLP error."
        exit 1
    fi
fi

exit 0
