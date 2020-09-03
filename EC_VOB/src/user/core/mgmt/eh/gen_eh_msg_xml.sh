#!/bin/bash

cd ${ACCPROJ:?"ACCPROJ is not defined. Please source build_env_init to init environment variables properly."}

EH_SRC_TOP_DIR=${ACCSRC}/user/core/mgmt/eh
EH_MSG_XML_INSTALL_DIR=${ACCROOTFS}/usr/eh
EH_TMP_DIR=${ACCPROJ}/eh_tmp

if [ ! -s ${EH_SRC_TOP_DIR}/include/eh_ui_msg_no.h ]; then
	echo "Error! eh_ui_msg_no.h is not found or the file is empty."
	exit 1
fi

[ -d ${EH_TMP_DIR} ] || mkdir -p ${EH_TMP_DIR}

if [ $? -ne 0 ]; then
	echo "An error occurs when creates temp directory for EH"
	exit 1
fi

cd ${EH_TMP_DIR}
cat > gen_macro_expand_output.c << EOF
#include <stdio.h>
#include "sys_module.h"
#include "eh_type.h"

EOF

egrep '^#define[ ]+EH_UI_MSG_NO_' ${EH_SRC_TOP_DIR}/include/eh_ui_msg_no.h > macro_expand.dat

echo "int value[]={" >> gen_macro_expand_output.c

awk '{print $2}' macro_expand.dat | sed 's/.*/&,/' >> gen_macro_expand_output.c

echo "};" >> gen_macro_expand_output.c

echo "char* name[]={" >> gen_macro_expand_output.c
awk '{print $2}' macro_expand.dat | sed 's/.*/"&",/' >> gen_macro_expand_output.c

echo "};" >> gen_macro_expand_output.c

cat >> gen_macro_expand_output.c << EOF
int main(void)
{
    int i;
    for (i=0; i<(sizeof(value)/sizeof(value[0])); i++)
        printf("#define %s %d\n", name[i], value[i]);
    return 0;
}
EOF

gcc -I${ACCSRC}/sysinclude -I${EH_SRC_TOP_DIR}/include gen_macro_expand_output.c -o gen_macro_expand_output
./gen_macro_expand_output > expand_ui_msg_no_define.h
if [ $? -ne 0 ]; then
    echo "An error occurs when compiles gen_macro_expand_output.c"
    exit 1
fi

rm -f eh_ui_msg_no.dtd > /dev/null 2>&1
echo '<?xml version="1.0" encoding="UTF-8"?>' > eh_ui_msg_no.dtd
awk '{printf("<!ENTITY %s \"%s\">\n",$2,$3);}' expand_ui_msg_no_define.h >> eh_ui_msg_no.dtd
if [ $? -ne 0 ]; then

    echo "An error occurs when generates eh_ui_msg_no.dtd"
    exit 1
fi

cp ${EH_SRC_TOP_DIR}/eh_msg_src.xml ./eh_msg_src.xml && xmllint --loaddtd --noent eh_msg_src.xml > eh_msg.xml
if [ $? -ne 0 ]; then
    echo "An error occurs when generates eh_msg.xml"
    exit 1
fi

install -m444 -D ./eh_msg.xml ${EH_MSG_XML_INSTALL_DIR}/eh_msg.xml
if [ $? -ne 0 ]; then
    echo "An error occurs when install eh_msg.xml to rootfs"
    exit 1
fi

echo "Generating eh_msg.xml done."
exit 0
