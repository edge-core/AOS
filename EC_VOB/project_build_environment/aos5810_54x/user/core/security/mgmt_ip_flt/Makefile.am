include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libmgmtipflt.la

libmgmtipflt_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/mgmt_ip_flt/mgmt_ip_flt.c

 libmgmtipflt_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include  \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include

CFLAGS += -Werror 

