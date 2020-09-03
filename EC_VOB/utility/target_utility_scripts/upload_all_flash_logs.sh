#!/bin/sh
# purpose: This script file is used to pack all kinds of log files on flash
#          into one file and upload to the specified TFTP server.
#
# arg1: tftp server ip address
#
# return: 0 if flash log archive file is uploaded to tftp server successfully
#         1 if failed to generate flash_logs.tar
#         2 if failed to upload through TFTP
#         3 if no archive file is uploaded because no log file exists on flash

#init variable archive_file_list
archive_file_list=

# function to pack the specified log directory to an archive file
# the archive file is output to /tmp
# arg 1: log directory name to be pack (directory name under /flash)
# arg 2: archive file name to be output to /tmp
# return: 0: No error occurs.
#         1: An error occurs in the execution of the function.
# note:
#        the archive file will be added to the variable "archive_file_list" if
#        the file is generated.
pack_log()
{
    local log_directory=$1
    local archive_file_name=$2

    # clear old archive file under /tmp if any
    if [ -f /tmp/${archive_file_name} ]; then
        rm /tmp/${archive_file_name}
    fi

    # check whether the log directory exists
    if [ -d /flash/${log_directory}/ ]; then
        tar zcf /tmp/${archive_file_name} /flash/${log_directory} 2>/dev/null 1>/dev/null
        if [ $? -eq "0" ]; then
            archive_file_list="$archive_file_list tmp/$archive_file_name"
        else
            echo "Failed to generate tmp/${archive_file_name}"
            return 1
        fi
    fi

    return 0
}

# change working directory to /
cd /

# pack exception log
pack_log exception_log exception_log.tgz

# pack software watchdog log
pack_log sw_wtd_log sw_wtd_log.tgz

# pack mref error log
pack_log mref_log mref_log.tgz

# archive all .tgz as flash_logs.tar if archive_file_list is not empty
if [ ! -z "${archive_file_list}" ]; then
    tar cf /tmp/flash_logs.tar ${archive_file_list}
    if [ $? -ne "0" ]; then
        echo "Failed to generate flash_logs.tar"
        exit 1
    fi

    cd /tmp
    tftp_command
    if [ $? -ne "0" ]; then
        echo "Failed to upload to tftp server $1."
        exit 2
    fi
    # archive file uploaded successfully
    exit 0
fi

# no archive file is generated
echo "No log file on flash"
exit 3
