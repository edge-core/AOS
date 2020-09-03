include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libudld_private.a
lib_LTLIBRARIES = libudld.la

libudld_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_engine.c \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_init.c \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_pmgr.c

libudld_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/udld/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


libudld_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/udld/udld_om.c

libudld_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/udld/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
