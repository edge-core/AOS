include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = liblandrv.la

liblandrv_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/driver/landrv/lan.c \
        $(ACCTONSRC_DIR)/user/driver/landrv/lan_om.c

liblandrv_la_CPPFLAGS = \
          $(COMMON_CPPFLAGS) \
        -I$(SDKKEN)/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include

CFLAGS += -Werror

