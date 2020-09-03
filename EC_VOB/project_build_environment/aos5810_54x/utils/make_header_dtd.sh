#! /bin/sh
OUT_ROOT=${ACCPROJ}/runtime
MGET=${ACCUTIL}/mget/bin/mget_x86_64

cat <<EOF >> ${OUT_ROOT}/temp.c
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#define MIB_MODULE "${MIB_MODULE}"
int main() { return 0; }
EOF

${MGET} -I${ACCSRC}/sysinclude/oem/${PROJECT_NAME}\
       -I${ACCSRC}/sysinclude\
       -I${ACCSRC}/sysinclude/mibconstants\
       -I/usr/include/linux\
       -I/usr/include\
       -I/usr/include/x86_64-linux-gnu\
       -o ${OUT_ROOT}/mibx.dtd -Of dtd -MiMIB_MODULE\
       ${OUT_ROOT}/temp.c

${MGET} -I${ACCSRC}/sysinclude/oem/${PROJECT_NAME}\
       -I${ACCSRC}/sysinclude\
       -I${ACCSRC}/sysinclude/mibconstants\
       -I/usr/include/linux\
       -I/usr/include\
       -I/usr/include/x86_64-linux-gnu\
       -o ${OUT_ROOT}/mibx.json -Of json -MiMIB_MODULE\
       ${OUT_ROOT}/temp.c

if [ $? != 0 ] ; then
        exit 1
fi

rm ${OUT_ROOT}/temp.c

