#!/bin/sh

MAX_LOG_NO=15

if [ ! -d /flash/exception_log ]; then
	mkdir /flash/exception_log
fi

if [ -f /flash/exception_log/next_log_no ]; then
	logno=`cat /flash/exception_log/next_log_no`
else
	logno=0
fi

LOG_FILE="/flash/exception_log/exception_log"${logno}".txt"
date > ${LOG_FILE}
if [ -f /etc/runtime_ver ]; then
	RUNTIME_VER=`cat /etc/runtime_ver`
else
	RUNTIME_VER="Unknown"
fi
# Append header to log file
echo "Runtime version:${RUNTIME_VER}" >> ${LOG_FILE}
opmode=`cat /flash/of_opmode.conf`
echo "of_opmode.conf=${opmode}" >> ${LOG_FILE}
echo "==================================================" >> ${LOG_FILE}
# redirect dmesg to log file
#dmesg >> ${LOG_FILE}

# Increase logno by 1
let logno=${logno}+1

# Check that whether or not need to rotate logno
if [ ${logno} = ${MAX_LOG_NO} ]; then
	logno=0
fi

# Update next log no
echo ${logno} > /flash/exception_log/next_log_no

echo ${LOG_FILE}

sync; sync
exit 0
