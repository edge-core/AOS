include $(top_srcdir)/commdef.mk

bin_PROGRAMS = sys_debug_proc

sys_debug_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/sys_debug_proc/sys_debug_proc.c 

sys_debug_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/process/sys_mgmt_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/bdpv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include

sys_debug_proc_LDADD = \
    $(ACCPROJ)/cmnlib/libcmnlib.la \
    $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
    $(top_builddir)/apps/backdoor/libbackdoor.la \
    $(top_builddir)/core/l3/netcfg/libnetcfg.la \
    $(top_builddir)/core/mgmt/syslog/libsyslog.la \
    $(top_builddir)/driver/devdrv/libdevdrv.la \
    $(top_builddir)/driver/swdrv/libswdrv.la \
    $(top_builddir)/core/l2/swctrl/libswctrl.la \
    $(top_builddir)/core/security/webauth/libwebauth.la \
    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
    $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
    $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
    $(top_builddir)/core/mgmt/wtdmgmt/libwtdmgmt.la \
    $(top_builddir)/driver/iscdrv/libiscdrv.la \
    $(top_builddir)/driver/landrv/liblandrv.la \
    $(top_builddir)/driver/sysdrv/libsysdrv.la \
    $(top_builddir)/driver/systime/libsystime.la \
    $(top_builddir)/apps/cli/libcli.la \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
    $(top_builddir)/core/cmgr/libcmgr.la \
    $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
    $(top_builddir)/core/l2/cn/libcn.la \
    $(top_builddir)/core/l2/pfc/libpfc.la \
    $(top_builddir)/core/l2/swctrl/libamtr.la \
    $(top_builddir)/core/l2/swctrl/rspan/librspan.la \
    $(top_builddir)/core/l2/vlan/libvlan.la \
    $(top_builddir)/core/l3/vxlan/libvxlan.la \
    $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
    $(top_builddir)/core/mgmt/debug/libdebug.la \
    $(top_builddir)/core/mgmt/sflow/libsflow.la \
    $(top_builddir)/core/mgmt/sysctrl/xor/libsysctrl_xor.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
    $(top_builddir)/driver/devdrv/onlp/libdevdrvonlp.la \
    $(top_builddir)/driver/fs/libfs.la \
    $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
    $(top_builddir)/driver/leddrv/libleddrv.la \
    $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
    $(top_builddir)/core/l3/amtrl3/libamtrl3.la \
    $(top_builddir)/core/l2/mlag/libmlag.la \
    $(top_builddir)/core/l2/lldp/liblldp.la \
    $(top_builddir)/core/l2/swctrl/lacp/liblacp.la \
    $(SDK_LIBS) \
    -lpthread

CFLAGS += -Werror
