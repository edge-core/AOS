FROM debian:9
MAINTAINER Bimal

# Install dependencies
RUN apt-get update

RUN apt-get update && apt-get clean \
    && apt-get install -y \
    autoconf \
    automake \
    libtool \
    build-essential \
    vim \
    bc \
    openssl \
    gawk \
    xsltproc \
    docbook-xsl \
    cpio \
    pkg-config \
    libncurses5-dev \
    bash-completion \
    bison \
    flex \
    libelf-dev \
    initramfs-tools \
    lvm2 \
    git \
    make \
    libjson-c3 \
    libreadline-dev \
    texinfo \
    libjson-c-dev \
    libc-ares-dev \
    python3-dev \
    python3-pytest \
    python3-sphinx \
    libsnmp-dev \
    libsystemd-dev \
    libcap-dev \
    swig \
    debhelper \
    cmake \
    libpcre3 \
    libpcre3-dev \
    doxygen \
    devscripts \
    rpm \
    graphviz \
    protobuf-compiler \
    protobuf-c-compiler \
    libprotobuf-c-dev \
    initramfs-tools-core \
    systemd

WORKDIR /

# Checkout 'libyang' & Install
RUN git clone https://github.com/CESNET/libyang.git

WORKDIR libyang
CMD mkdir build
WORKDIR build
RUN pwd
RUN cmake -DENABLE_LYD_PRIV=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr -D CMAKE_BUILD_TYPE:String="Release" ..
RUN make
RUN make install

CMD cd /

# /bin/sh points to Dash by default, reconfigure to use bash until Android
# build becomes POSIX compliant
RUN echo "dash dash/sh boolean false" | debconf-set-selections && \
    dpkg-reconfigure -p critical dash

#RUN ln -s /usr/bin/env /bin/env

#RUN cd /usr/bin/ && ln -s gcc-ar-4.7 x86_64-linux-gnu-ar
#RUN cd /usr/bin/ && ln -s gcc-ar-4.7 x86_64-linux-gnu-ar && \
#                    ln -s ld x86_64-linux-gnu-ld && \
#                    ln -s nm x86_64-linux-gnu-nm && \
#                    ln -s objcopy x86_64-linux-gnu-objcopy



RUN sed -i '/define _ASM_X86_UNISTD_64_H/a \\n#define __NR_sys_SYSFUN     335 /* Accton Patch */' \
/usr/include/x86_64-linux-gnu/asm/unistd_64.h

RUN sed -i '/ifdef __USE_MISC/a \\n/* ACCTON PATCH */\n#define MSG_SETOWNER 15  /* set msgq owner */ \
\n#define MSG_GETOWNER 16  /* get msgq owner */' /usr/include/x86_64-linux-gnu/bits/msq.h 

