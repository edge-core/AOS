include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libqos_v2.la

noinst_LIBRARIES = libqos_v2_private.a

libqos_v2_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/qos_v2/l4_mgr.c \
    $(ACCTONSRC_DIR)/user/core/qos_v2/l4_init.c \
    $(ACCTONSRC_DIR)/user/core/qos_v2/cos/cos_vm.c \
    $(ACCTONSRC_DIR)/user/core/qos_v2/rule_mgr/rule_mgr.c \
    $(ACCTONSRC_DIR)/user/core/qos_v2/l4_backdoor.c

libqos_v2_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(SDK)/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/rule_mgr/include

libqos_v2_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/qos_v2/l4_pmgr.c

libqos_v2_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include
