#!/bin/sh

#start of RD debug information
RD_DEBUG=0
DB_LIMIT=50
if [ $RD_DEBUG -eq 1 ]; then
    if [ ! -d /tmp/ip ]; then
        mkdir /tmp/ip
    fi
{
    echo "IF_CMD=${IF_CMD}"
    echo "IF_COMM=${IF_COMM}"
    echo "IF_NAME=${IF_NAME}"
    echo "IF_FAMI=${IF_FAMI}"
    echo "IF_ADDR=${IF_ADDR}"
    echo "IF_MASK=${IF_MASK}"
    #set
} > /tmp/ip/cfg_${IF_PID}_${IF_CMD}

cd /tmp/ip && ls -t | tail -n +$(($DB_LIMIT + 1)) | xargs rm -f

fi
#end of RD debug information

/usr/bin/netcfg_ip_proc
echo $? > "/proc/umh_res_${IF_PID}_${IF_CMD}"

