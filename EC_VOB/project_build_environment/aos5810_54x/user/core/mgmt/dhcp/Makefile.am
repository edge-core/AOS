include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdhcp_private.a

lib_LTLIBRARIES = libdhcp.la

libdhcp_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_backdoor.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_init.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_mgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_type.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/alloc.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/alloc.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/bootp.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/bootp.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_algo.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_algo.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_error_print.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_error_print.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_om_private.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_time.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_time.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_txrx.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_txrx.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_wa.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_wa.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/hash.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/hash.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/memory.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/memory.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/tables.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/tree.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/tree.h

libdhcp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
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
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants

libdhcp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcp/dhcp_pom.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include/dhcp_pom.h
	
libdhcp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include

libdhcpsnp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include

CFLAGS += -Werror -Wno-misleading-indentation
