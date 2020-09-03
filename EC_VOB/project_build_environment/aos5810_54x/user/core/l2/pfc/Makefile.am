include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libpfc_private.a
lib_LTLIBRARIES = libpfc.la

libpfc_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_init.c \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_pmgr.c

libpfc_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/flashdrv/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


libpfc_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l2/pfc/pfc_om.c

libpfc_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include

CFLAGS += -Werror
