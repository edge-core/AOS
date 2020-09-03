#!/bin/bash

if [ $# -ne 2 ] ; then
	echo "Generate empty dummy functions from the specified compile"
	echo "error log which contains undefined reference function."
	echo ""
	echo "Usage: $0 <compiled error log> <output_dummy_func_filename>"
	exit 1
fi

compiled_log_file=$1
output_file=$2

cat ${compiled_log_file} | grep "undefined reference to "'`'"[A-Za-z0-9_]*""'""$" | \
    sed 's/^.*'"\`"'//' | sed 's/'"'"'$//' | sort -u | sed 's/.*/void &(void){return;}/' > ${output_file}

echo "${output_file} is generated."

exit 0
