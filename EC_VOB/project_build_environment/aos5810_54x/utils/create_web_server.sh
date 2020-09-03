#!/bin/sh

DIR=${ACCPROJ}/utils

echo "Copy HTML files ..."
${DIR}/cp_html_files.sh
if [ $? != 0 ] ; then
        echo "Failed."
        exit 1
fi

echo "Make mibx.dtd..."
${DIR}/make_header_dtd.sh
if [ $? != 0 ] ; then
        echo "Failed."
        exit 1
fi

#echo "Build WEB UI..."
#${DIR}/build_web_ui.sh
#if [ $? != 0 ] ; then
#        echo "Failed."
#        exit 1
#fi
