include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libpppoe_ia_private.a
lib_LTLIBRARIES = libpppoe_ia.la

libpppoe_ia_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_om.c \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_engine.c \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_init.c \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_pmgr.c 

libpppoe_ia_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include

libpppoe_ia_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/pppoe_ia_pmgr.c

libpppoe_ia_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include
