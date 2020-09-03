include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsysrsc_private.a

lib_LTLIBRARIES=libsysrsc.la

libsysrsc_private_a_SOURCES=$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/sysrsc_mgr_private.c

libsysrsc_private_a_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include  \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/syncedrv/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/privatevlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include/esw \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(SDKKEN)/include

libsysrsc_la_SOURCES=$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/sysrsc_mgr.c

libsysrsc_la_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/core/cmgr/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/syncedrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/privatevlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include/esw \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(SDKKEN)/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
