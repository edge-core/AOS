include $(top_builddir)/commdef.mk

lib_LTLIBRARIES=libsys_callback.la

libsys_callback_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/sys_callback_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/sys_callback_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/sys_callback_refined_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/sys_callback_mgr.c

libsys_callback_la_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include

CFLAGS += -Werror

