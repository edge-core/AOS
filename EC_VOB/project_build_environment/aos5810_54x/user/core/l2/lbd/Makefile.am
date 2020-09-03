include $(top_builddir)/commdef.mk


noinst_LIBRARIES = liblbd_private.a

liblbd_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/lbd/lbd_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/lbd/lbd_mgr.c

liblbd_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include


lib_LTLIBRARIES = liblbd.la

liblbd_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/lbd/lbd_om.c \
    $(ACCTONSRC_DIR)/user/core/l2/lbd/lbd_pmgr.c

liblbd_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include


CFLAGS += -Werror
