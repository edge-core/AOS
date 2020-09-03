include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdhcpsnp_private.a

lib_LTLIBRARIES = libdhcpsnp.la

libdhcpsnp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_engine.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_engine.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_task.c

libdhcpsnp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
        -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/ \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
	-I$(ACCTONSRC_DIR)/user/core/qos/acl/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
	-I$(ACCTONSRC_DIR)/user/process/ip_service_proc/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants

libdhcpsnp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcpsnp_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcpsnp_pmgr.c
	
libdhcpsnp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
