include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libcn_private.a

libcn_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_engine.c \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_mgr.c

libcn_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/ets/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include


lib_LTLIBRARIES = libcn.la

libcn_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_om.c \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/cn/cn_pom.c

libcn_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include


CFLAGS += -Werror
