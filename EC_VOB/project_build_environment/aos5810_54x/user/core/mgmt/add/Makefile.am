include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libadd_private.a
lib_LTLIBRARIES = libadd.la

libadd_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/add_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/linux/add_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/add_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/add_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/add_backdoor.c

libadd_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include 
    
libadd_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/add/add_pmgr.c

libadd_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include

CFLAGS += -Werror -D LINUX_FRAMER
