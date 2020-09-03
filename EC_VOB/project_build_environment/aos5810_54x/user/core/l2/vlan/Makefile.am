
include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libvlan_private.a

libvlan_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_backdoor.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_type.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_om_private.h

libvlan_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


lib_LTLIBRARIES = libvlan.la

libvlan_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_lib.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/include/vlan_pom.h \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_lib.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/vlan/vlan_pom.c

libvlan_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
