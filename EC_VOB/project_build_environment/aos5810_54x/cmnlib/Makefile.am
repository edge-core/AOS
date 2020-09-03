SUBDIRS = netopeer

lib_LTLIBRARIES = libcmnlib.la

libcmnlib_la_SOURCES = \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_base64.c \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_bitmap.c \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_cvrt.c \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_inet.c \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_math.c \
	$(ACCTONSRC_DIR)/cmnlib/common/convert/l_md5.c \
        $(ACCTONSRC_DIR)/cmnlib/common/datastru/l_avl.c \
        $(ACCTONSRC_DIR)/cmnlib/common/datastru/l_avl_util.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_cirlst.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_dlist.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_hash.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_hisam.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_isam.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_link_lst.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_lst.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_merge_sort.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_mq.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_msg.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_openhash.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_sort_lst.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_stack.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_radix.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_ls_table.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_ls_prefix.c \
	$(ACCTONSRC_DIR)/cmnlib/common/memmgmt/l_memgc.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_linklist.c \
	$(ACCTONSRC_DIR)/cmnlib/common/datastru/l_ptree.c \
	$(ACCTONSRC_DIR)/cmnlib/common/memmgmt/l_mpool.c \
	$(ACCTONSRC_DIR)/cmnlib/common/memmgmt/l_pt.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_hold_timer.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_mname.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_rstatus.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_stdlib.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_string.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_timer.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_prefix.c \
	$(ACCTONSRC_DIR)/cmnlib/user/kernel/linux/sysfun.c \
	$(ACCTONSRC_DIR)/cmnlib/user/memmgmt/l_ipcmem.c \
	$(ACCTONSRC_DIR)/cmnlib/user/memmgmt/l_mm.c \
	$(ACCTONSRC_DIR)/cmnlib/user/l_cmnlib_init.c \
	$(ACCTONSRC_DIR)/cmnlib/user/l_ipcio.c \
	$(ACCTONSRC_DIR)/cmnlib/user/l_threadgrp.c \
	$(ACCTONSRC_DIR)/cmnlib/common/l_charset.c \
	$(ACCTONSRC_DIR)/cmnlib/user/memmgmt/l_mm_backdoor.c

libcmnlib_la_CPPFLAGS = \
	-DLIBC_USE_GNU \
	-DSYSCALL_DECLARATION_STYLE_2 \
	-I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
 	-I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I${PROJECT_PATH}/user/thirdpty/xml/libxml2-2.9.0/include/libxml2 \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include

