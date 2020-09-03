include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdhcpv6snp_private.a

lib_LTLIBRARIES = libdhcpv6snp.la

libdhcpv6snp_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_engine.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_task.c

libdhcpv6snp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
        -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/iml \
        -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants

libdhcpv6snp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6snp_pom.c
libdhcpv6snp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
