include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libmlag_private.a

libmlag_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/mlag/mlag_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/mlag/mlag_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/mlag/mlag_om.c

libmlag_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/mlag/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include


lib_LTLIBRARIES = libmlag.la

libmlag_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/mlag/mlag_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/mlag/mlag_pom.c

libmlag_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/mlag/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include


CFLAGS += -Wall -Werror
