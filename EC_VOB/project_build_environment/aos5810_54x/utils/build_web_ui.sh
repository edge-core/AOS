#! /bin/sh
WEB_ROOT=${ACCROOTFS}/usr/webroot
OUT_ROOT=${ACCPROJ}/runtime
SRC_ROOT=${ACCSRC}/user/apps/web/html

XT=xsltproc
SRC_FILE=web-ui.xml
OUTPUT_FILE=web-ui.js

#cp -f ${SRC_ROOT}/${SRC_FILE} ${OUT_ROOT}/${SRC_FILE}

${XT} --xinclude --output ${OUT_ROOT}/temp.xml \
  ${ACCUTIL}/xslt/apply-opt-cond.xsl \
  ${OUT_ROOT}/${SRC_FILE}

if [ $? != 0 ] ; then
        echo "Convert temp.xml failed"
        rm ${OUT_ROOT}/temp.xml
        rm ${OUT_ROOT}/${SRC_FILE}
        exit 1
fi

${XT} --nonet --xinclude --output ${WEB_ROOT}/js/${OUTPUT_FILE} \
  ${ACCUTIL}/xslt/jsonx-to-jsonp.xsl \
  ${OUT_ROOT}/temp.xml

if [ $? != 0 ] ; then
        echo "Convert web-ui.js failed"
        rm ${OUT_ROOT}/temp.xml
        rm ${OUT_ROOT}/${SRC_FILE}
        exit 1
fi

rm ${OUT_ROOT}/temp.xml
rm -f ${OUT_ROOT}/${SRC_FILE}
