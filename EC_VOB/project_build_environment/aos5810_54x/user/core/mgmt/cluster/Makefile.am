include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libcluster.la

noinst_LIBRARIES = libcluster_private.a

libcluster_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_om.c


libcluster_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/process/sys_mgmt_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include

libcluster_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/cluster/cluster_pom.c

libcluster_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include

CFLAGS += -Werror
