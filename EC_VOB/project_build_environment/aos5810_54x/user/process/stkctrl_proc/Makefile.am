include $(top_srcdir)/commdef.mk

bin_PROGRAMS = stkctrl_proc
stkctrl_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/stkctrl_proc/stkctrl_proc.c \
    $(ACCTONSRC_DIR)/user/process/stkctrl_proc/stkctrl_proc_comm.c \
    $(ACCTONSRC_DIR)/user/process/stkctrl_proc/stkctrl_group.c \
    $(ACCTONSRC_DIR)/user/process/driver_proc/driver_group_operation.c

stkctrl_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/process/stkctrl_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscagent/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/process/driver_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include/ \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include/ \
    -I$(ACCTONSRC_DIR)/user/driver/msl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/msl/mslt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/syncedrv/include \
    -I$(SDKKEN)/include

stkctrl_proc_LDADD = \
	$(COMMON_LIBS) \
    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt_private.a \
    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
    $(top_builddir)/apps/cli/libcli.la \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
    $(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
    $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/core/security/appfilter/libappfilter.la \
    $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
    $(top_builddir)/driver/sysdrv/libsysdrv.la \
    $(top_builddir)/driver/systime/libsystime.la \
    $(top_builddir)/driver/iscdrv/libiscdrv.la \
    $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
    $(top_builddir)/core/mgmt/syslog/libsyslog.la \
    $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
    $(top_builddir)/core/mgmt/sysmgmt/libsysmgmt.la \
    $(top_builddir)/core/mgmt/telnet/libtelnet.la \
    $(top_builddir)/driver/swdrv/libswdrv.la \
    $(top_builddir)/driver/systime/libsystime.la \
    $(top_builddir)/core/l2/swctrl/libamtr.la \
    $(top_builddir)/core/l2/swctrl/libswctrl.la \
    $(top_builddir)/core/l2/vlan/libvlan.la \
    $(top_builddir)/core/l3/netcfg/libnetcfg.la \
    $(top_builddir)/driver/devdrv/libdevdrv.la \
    $(top_builddir)/driver/fs/libfs.la \
    $(top_builddir)/apps/backdoor/libbackdoor.la \
    $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
    $(top_builddir)/driver/iscdrv/libiscdrv.la \
    $(top_builddir)/driver/iscagent/libiscagent.la \
    $(top_builddir)/driver/landrv/liblandrv.la \
    $(top_builddir)/driver/leddrv/libleddrv.la \
    $(ACCPROJ)/cmnlib/libcmnlib.la \
    $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
    $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
    $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
    -lpthread

CFLAGS += -Werror
