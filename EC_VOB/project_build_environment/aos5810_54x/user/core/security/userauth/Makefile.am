include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libuserauth_private.a

lib_LTLIBRARIES = libuserauth.la

libuserauth_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/userauth/userauth.c

libuserauth_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include

libuserauth_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/userauth/userauth_pmgr.c

libuserauth_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include

CFLAGS += -Werror 
