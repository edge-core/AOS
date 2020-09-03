include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libets_private.a

lib_LTLIBRARIES = libets.la


libets_private_a_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/ets/ets_backdoor.c \
        $(ACCTONSRC_DIR)/user/core/l2/ets/ets_init.c \
        $(ACCTONSRC_DIR)/user/core/l2/ets/ets_mgr.c \
        $(ACCTONSRC_DIR)/user/core/l2/ets/ets_om.c

libets_private_a_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/ets/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/security/include \
        -I$(ACCTONSRC_DIR)/user/core/security/dot1x \
        -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


libets_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/ets/ets_pmgr.c

libets_la_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/l2/ets/include

CFLAGS += -Werror
