include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libdns_private.a
lib_LTLIBRARIES = libdns.la

libdns_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_cache.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_cmm.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_hostlib.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_inet.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_list.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_proxy.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_resolver.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_vm.c
    
libdns_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/process/core_util_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include

libdns_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/dns/dns_pom.c

libdns_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include

# TBD:Turn off -Werror until all compile warnings are fixed
AM_CFLAGS = -Wno-error 
