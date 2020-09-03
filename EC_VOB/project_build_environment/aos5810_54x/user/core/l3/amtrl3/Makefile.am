include $(top_builddir)/commdef.mk

noinst_LIBRARIES   = libamtrl3_private.a
lib_LTLIBRARIES = libamtrl3.la

libamtrl3_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_om.c \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_init.c \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_task.c \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_backdoor.c 

libamtrl3_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3 \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/security/sshv2 \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/kernel/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/common/convert \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/sysinclude/oem/${PROJECT_NAME} \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include
	
libamtrl3_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/amtrl3/amtrl3_pom.c 

libamtrl3_la_CPPFLAGS=\
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include

AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format
