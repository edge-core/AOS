include $(top_builddir)/commdef.mk

noinst_LIBRARIES= libtelnet_private.a
lib_LTLIBRARIES = libtelnet.la

libtelnet_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnet_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnet_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnet_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnetd.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/tnshd.c
#    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/dcli.c

libtelnet_private_a_CFLAGS = -gdwarf-2

libtelnet_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/apps/cli \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include

libtelnet_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnet_pom.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/telnet/telnet_pmgr.c
    
libtelnet_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/debug/include

CFLAGS += -Werror
