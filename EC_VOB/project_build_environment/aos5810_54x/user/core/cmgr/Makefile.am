include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libcmgr.la

libcmgr_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/cmgr/cmgr.c

libcmgr_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/cmgr/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/oam/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
