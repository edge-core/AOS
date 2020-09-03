include $(top_srcdir)/commdef.mk

bin_PROGRAMS = app_protocol_proc
app_protocol_proc_SOURCES = \
	$(ACCTONSRC_DIR)/user/process/app_protocol_proc/app_protocol_proc.c \
	$(ACCTONSRC_DIR)/user/process/app_protocol_proc/app_protocol_group.c \
	$(ACCTONSRC_DIR)/user/process/app_protocol_proc/app_protocol_proc_comm.c

app_protocol_proc_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ping/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/traceroute/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
	-I$(ACCTONSRC_DIR)/user/process/app_protocol_proc/include

app_protocol_proc_LDADD = \
	$(top_builddir)/core/mgmt/sntp/libsntp_private.a

app_protocol_proc_LDADD += \
	$(COMMON_LIBS) \
	$(top_builddir)/apps/backdoor/libbackdoor.la \
	$(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
	$(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
	$(ACCPROJ)/cmnlib/libcmnlib.la \
	$(top_builddir)/core/l2/bridge/xstp/libxstp.la \
        $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
	$(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
	$(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
	$(top_builddir)/core/mgmt/syslog/libsyslog.la \
	$(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
	$(top_builddir)/core/security/appfilter/libappfilter.la \
	$(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
	$(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
	$(top_builddir)/core/mgmt/dns/libdns_private.a \
	$(top_builddir)/core/mgmt/sysmgmt/libsysmgmt.la \
   	$(top_builddir)/core/mgmt/telnet/libtelnet.la \
	$(top_builddir)/core/mgmt/ntp/libntp_private.a \
	$(top_builddir)/core/l2/swctrl/libamtr.la \
	$(top_builddir)/core/l2/swctrl/libswctrl.la \
	$(top_builddir)/core/l2/vlan/libvlan.la \
	$(top_builddir)/core/l3/netcfg/libnetcfg.la	\
	$(top_builddir)/core/l3/ping/libping_private.a \
	$(top_builddir)/core/l3/ping/libping.la	\
	$(top_builddir)/core/l3/traceroute/libtraceroute_private.a	\
	$(top_builddir)/core/l3/traceroute/libtraceroute.la	\
	$(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
	$(top_builddir)/driver/devdrv/libdevdrv.la \
	$(top_builddir)/driver/fs/libfs.la \
	$(top_builddir)/driver/i2cdrv/libi2cdrv.la \
	$(top_builddir)/driver/iscdrv/libiscdrv.la \
	$(top_builddir)/driver/landrv/liblandrv.la \
	$(top_builddir)/driver/leddrv/libleddrv.la \
	$(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
	$(top_builddir)/driver/swdrv/libswdrv.la \
	$(top_builddir)/driver/sysdrv/libsysdrv.la \
	$(top_builddir)/driver/systime/libsystime.la \
	-lpthread

CFLAGS += -Werror 

