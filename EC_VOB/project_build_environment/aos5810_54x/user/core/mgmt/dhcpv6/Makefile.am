include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdhcpv6_private.a

lib_LTLIBRARIES = libdhcpv6.la

libdhcpv6_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include/dhcpv6_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include/dhcpv6_init.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include/dhcp6.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include/cfg.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_algo.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/timer.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/timer.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/lease.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/lease.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/queue.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/common.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/common.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcp6c.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcp6r.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/relay6_database.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/relay6_parser.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/relay6_socket.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/client6_addr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_mgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_om.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_pom.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_mgr.c

libdhcpv6_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
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
	-I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
	-I$(ACCTONSRC_DIR)/cmmlib/common/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/prom/loader/u-boot-1.3.2/include \
	-I$(ACCTONSRC_DIR)/user/process/ip_service_proc/include

libdhcpv6_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include/dhcp_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/dhcpv6_pom.c
	
libdhcpv6_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include

libdhcpv6snp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
