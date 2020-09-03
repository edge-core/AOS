include $(top_srcdir)/commdef.mk

bin_PROGRAMS = snmp_proc

snmp_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/snmp_proc/snmp_proc.c \
    $(ACCTONSRC_DIR)/user/process/snmp_proc/snmp_proc_comm.c \
    $(ACCTONSRC_DIR)/user/process/snmp_proc/snmp_group.c

snmp_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/process/snmp_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/process/cli_proc/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt_v3/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/ \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/privatevlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/sync_e/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/ets/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/igmpcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/mldcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ping/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp \
    -I$(ACCTONSRC_DIR)/user/core/l3/traceroute/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/protocol \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/dcbx/include

snmp_proc_LDADD = \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3_private.a \
    $(top_builddir)/thirdpty/snmp/net-snmp-5.1/libnetnet_snmp_5_1_private.a \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3_private.a \
    $(COMMON_LIBS) \
    $(top_builddir)/apps/cli/libcli.la \
    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
    $(top_builddir)/core/l2/swctrl/lacp/liblacp.la \
    $(top_builddir)/core/l2/bridge/extbrg/libextbrg.la \
    $(top_builddir)/core/l2/bridge/pri_mgmt/libprimgmt.la \
    $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
    $(top_builddir)/core/l2/dcbx/libdcbx.la \
    $(top_builddir)/core/l2/ets/libets.la \
    $(top_builddir)/core/l2/l2mux/libl2mux.la \
    $(top_builddir)/core/l2/lldp/liblldp.la \
    $(top_builddir)/core/l2/swctrl/libamtr.la \
    $(top_builddir)/core/l2/swctrl/libnmtr.la \
    $(top_builddir)/core/l2/swctrl/libswctrl.la \
    $(top_builddir)/core/l2/swctrl/libtrunk.la \
    $(top_builddir)/core/l2/vlan/libvlan.la \
    $(top_builddir)/core/l2/pppoe_ia/libpppoe_ia.la \
    $(top_builddir)/core/l3/ipal/libipal.la \
    $(top_builddir)/core/l3/iml/libiml.la \
    $(top_builddir)/core/l3/udphelper/libudphelper.la \
    $(top_builddir)/core/l3/netcfg/libnetcfg.la \
    $(top_builddir)/core/l3/ping/libping.la \
    $(top_builddir)/core/l3/vrrp/libvrrp.la \
    $(top_builddir)/core/l3/traceroute/libtraceroute.la \
    $(top_builddir)/core/l3/traceroute/libtraceroute.la \
    $(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
    $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
    $(top_builddir)/core/mgmt/sntp/libsntp.la \
    $(top_builddir)/core/mgmt/ntp/libntp.la \
    $(top_builddir)/core/mgmt/smtp/libsmtp.la \
    $(top_builddir)/core/mgmt/syslog/libsyslog.la \
    $(top_builddir)/core/mgmt/sysmgmt/libsysmgmt.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
    $(top_builddir)/core/mgmt/xfer/libxfer.la \
    $(top_builddir)/core/mgmt/http/libhttp.la \
    $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
    $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
    $(top_builddir)/core/mgmt/dns/libdns.la \
    $(top_builddir)/core/mgmt/add/libadd.la \
    $(top_builddir)/core/mgmt/sflow/libsflow.la\
    $(top_builddir)/core/mgmt/dhcp/libdhcp.la \
    $(top_builddir)/core/mgmt/telnet/libtelnet.la \
    $(top_builddir)/core/qos_v2/libqos_v2.la \
    $(top_builddir)/core/security/portsec/libportsec.la \
    $(top_builddir)/core/security/userauth/libuserauth.la \
    $(top_builddir)/core/security/tacacs/libtacacs.la \
    $(top_builddir)/core/security/webauth/libwebauth.la \
    $(top_builddir)/core/security/keygen/libkeygen.la \
    $(top_builddir)/core/security/dot1x/libdot1x.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/core/security/sshv2/libssh.la \
    $(top_builddir)/core/security/radius/libradius.la \
    $(top_builddir)/core/security/libaaa.la \
    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
    $(top_builddir)/core/security/netaccess/libnetaccess.la \
    $(top_builddir)/core/security/appfilter/libappfilter.la \
    $(top_builddir)/driver/devdrv/libdevdrv.la \
    $(top_builddir)/driver/fs/libfs.la \
    $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
    $(top_builddir)/driver/iscdrv/libiscdrv.la \
    $(top_builddir)/driver/landrv/liblandrv.la \
    $(top_builddir)/driver/leddrv/libleddrv.la \
    $(top_builddir)/driver/swdrv/libswdrv.la \
    $(top_builddir)/driver/sysdrv/libsysdrv.la \
    $(top_builddir)/driver/systime/libsystime.la \
    $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
    $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
    -lpthread

#snmp_proc_LIBTOOLFLAGS = --preserve-dup
AM_LIBTOOLFLAGS = --preserve-dup-deps

CFLAGS += -Werror
