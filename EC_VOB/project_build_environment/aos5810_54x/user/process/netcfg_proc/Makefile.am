include $(top_srcdir)/commdef.mk

bin_PROGRAMS = netcfg_proc netcfg_ip_proc

netcfg_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/netcfg_proc/netcfg_proc.c \
    $(ACCTONSRC_DIR)/user/process/netcfg_proc/netcfg_proc_comm.c \
    $(ACCTONSRC_DIR)/user/process/netcfg_proc/netcfg_group.c \
    $(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_debug.c \
    $(ACCTONSRC_DIR)/user/core/l3/ipal/linux/ipal_reflect.c

netcfg_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/process/netcfg_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/igv3snp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include/ \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/igmpcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/mldcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/protocol \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include

netcfg_proc_LDADD = \
	$(COMMON_LIBS) \
    $(top_builddir)/core/l3/netcfg/libnetcfg_private.a \
    $(top_builddir)/apps/backdoor/libbackdoor.la \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
    $(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
    $(ACCPROJ)/cmnlib/libcmnlib.la \
    $(top_builddir)/core/l3/netcfg/libnetcfg_private.a \
    $(top_builddir)/core/l2/l2mux/libl2mux.la \
    $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
    $(top_builddir)/core/l2/swctrl/libamtr.la \
    $(top_builddir)/core/l2/swctrl/libnmtr.la \
    $(top_builddir)/core/l2/swctrl/libswctrl.la \
    $(top_builddir)/core/l2/vlan/libvlan.la \
    $(top_builddir)/core/l3/iml/libiml.la \
    $(top_builddir)/core/l3/vrrp/libvrrp.la \
    $(top_builddir)/core/l3/ipal/libipal.la \
    $(top_builddir)/core/l3/udphelper/libudphelper.la \
    $(top_builddir)/core/l3/netcfg/libnetcfg.la \
    $(top_builddir)/core/l3/vrrp/libvrrp.la \
    $(top_builddir)/core/mgmt/dhcp/libdhcp.la \
    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
    $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
    $(top_builddir)/core/mgmt/syslog/libsyslog.la \
    $(top_builddir)/core/mgmt/sysmgmt/libsysmgmt.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/core/security/appfilter/libappfilter.la \
    $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
    $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
    $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
    $(top_builddir)/core/mgmt/telnet/libtelnet.la \
    $(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
    $(top_builddir)/core/l2/lldp/liblldp.la \
    $(top_builddir)/driver/devdrv/libdevdrv.la \
    $(top_builddir)/core/l2/swctrl/libtrunk.la \
    $(top_builddir)/core/qos_v2/libqos_v2.la \
    $(top_builddir)/driver/fs/libfs.la \
    $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
    $(top_builddir)/driver/landrv/liblandrv.la \
    $(top_builddir)/driver/iscdrv/libiscdrv.la \
    $(top_builddir)/driver/leddrv/libleddrv.la \
    $(top_builddir)/driver/swdrv/libswdrv.la \
    $(top_builddir)/driver/sysdrv/libsysdrv.la \
    $(top_builddir)/driver/systime/libsystime.la \
    $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
    $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
    -lpthread

netcfg_ip_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/netcfg_proc/netcfg_ip_proc.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_pmgr_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_pom_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ip_lib.c \
    $(ACCTONSRC_DIR)/cmnlib/user/kernel/linux/sysfun.c \
    $(ACCTONSRC_DIR)/cmnlib/common/convert/l_inet.c \
    $(ACCTONSRC_DIR)/cmnlib/common/l_charset.c

netcfg_ip_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -DLIBC_USE_GNU \
    -DSYSCALL_DECLARATION_STYLE_2 \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/sysinclude

netcfg_ip_proc_LDADD = \
    -lpthread

AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format
