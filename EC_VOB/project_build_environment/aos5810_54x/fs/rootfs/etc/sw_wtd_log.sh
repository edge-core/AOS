#!/bin/sh

# arg 1:thread id
# arg 2:thread name

MAX_LOG_NO=15

if [ $# -ne 2 ]; then
    echo "Incorrect arguments."
    exit 1
fi

if [ ! -d /flash/sw_wtd_log ]; then
        mkdir /flash/sw_wtd_log
fi

if [ -f /flash/sw_wtd_log/next_log_no ]; then
        logno=`cat /flash/sw_wtd_log/next_log_no`
else
        logno=0
fi

# generate software watchdog log filename
LOG_FILE="/flash/sw_wtd_log/sw_wtd_log"${logno}".txt"
# Add date and version to logfile
date > ${LOG_FILE}
if [ -f /etc/runtime_ver ]; then
        RUNTIME_VER=`cat /etc/runtime_ver`
else
        RUNTIME_VER="Unknown"
fi
echo "Runtime version:${RUNTIME_VER}" >> ${LOG_FILE}
echo "==================================================" >> ${LOG_FILE}
echo "Timeout thread id:$1 thread name:$2" >> ${LOG_FILE}
echo "" >> ${LOG_FILE}

# Check whether the thread id exists
if [ ! -f /proc/$1/ustack ]; then
echo "Thread id $1 not exists." >> ${LOG_FILE}
else
# Due to the special implementation in /proc/$1/ustack, the out is not able to redirect
# So a quick way to capture the output is to redirect the content of dmesg
cat /proc/$1/ustack >> ${LOG_FILE}
dmesg >> ${LOG_FILE}
cat /proc/$1/maps >> ${LOG_FILE}
echo "cat /proc/$1/wchan" >> ${LOG_FILE}
cat /proc/$1/wchan >> ${LOG_FILE}
fi

# Increase logno by 1
let logno=${logno}+1

# Check that whether or not need to rotate logno
if [ ${logno} = ${MAX_LOG_NO} ]; then
        logno=0
fi

# Update next log no
echo ${logno} > /flash/sw_wtd_log/next_log_no
exit 0
