#!/bin/sh

# arg 1:previous caller addr when mref corruption is detected
# arg 2:previous caller tid when mref corruption is detected

MREF_LOG_DIR='/flash/mref_log'
MAX_LOG_NO=100

if [ ! -d ${MREF_LOG_DIR} ]; then
    mkdir ${MREF_LOG_DIR}
fi 

if [ -f /etc/runtime_ver ]; then
    RUNTIME_VER=`cat /etc/runtime_ver`
else
    RUNTIME_VER="Unknown"
fi

if [ -f ${MREF_LOG_DIR}/mref_log_next_log_no ]; then
    logno=`cat ${MREF_LOG_DIR}/mref_log_next_log_no`
else
    logno=0
fi

let logno=${logno}+1
LOG_FILE="${MREF_LOG_DIR}/mref_log${logno}.txt"
if [ ${logno} = ${MAX_LOG_NO} ]; then
    logno=0
fi
# Update next log no
echo ${logno} > ${MREF_LOG_DIR}/mref_log_next_log_no


echo "==================================================" > ${LOG_FILE}
echo "Runtime version:${RUNTIME_VER}" >> ${LOG_FILE}
# Add date and version to logfile
date >> ${LOG_FILE}
echo "==================================================" >> ${LOG_FILE}
echo "Previous caller addr:0x$1 tid:$2" >> ${LOG_FILE}
dmesg >> ${LOG_FILE}

exit 0
