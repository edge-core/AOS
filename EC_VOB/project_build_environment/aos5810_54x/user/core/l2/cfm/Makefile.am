include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libcfm_private.a
lib_LTLIBRARIES = libcfm.la

libcfm_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_timer.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_engine.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_pmgr.c 

libcfm_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
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
	-I$(ACCTONSRC_DIR)/user/driver/flashdrv/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include

libcfm_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_timer.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/cfm/cfm_pmgr.c

libcfm_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include

AM_CFLAGS = -Werror
