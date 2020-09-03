include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libdcbx_private.a

lib_LTLIBRARIES = libdcbx.la


libdcbx_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/include/dcbx_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/include/dcbx_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/include/dcbx_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/include/dcbx_type.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_backdoor.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_om_private.h

libdcbx_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/dcbx/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/ets/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/core/security/dot1x \
	-I$(ACCTONSRC_DIR)/user/core/security/netaccess \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


libdcbx_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/include/dcbx_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/dcbx/dcbx_pmgr.c


libdcbx_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/dcbx/include

CFLAGS += -Werror
