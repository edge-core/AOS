include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsnmpmgmt_v3_private.a

lib_LTLIBRARIES = libsnmpmgmt_v3.la

libsnmpmgmt_v3_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_task.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_mgr.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_trap_queue.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_init.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_cluster.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/netsnmp_port.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/es3626a_superset2.0.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_arp.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_rip.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_ospf.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_ets.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_pfc.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_ecmp.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/l_snmp.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/nlm_mgr.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/nlm.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/oem/${PROJECT_NAME}/mibtreeinit.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/acl_v2.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/mibgroup/mib_synce.c


libsnmpmgmt_v3_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include/mibgroup \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/oem \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cn/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/dcbx/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/igv3snp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/mvr6/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lbd/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/privatevlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/oam/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/ets/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pfc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/pppoe_ia/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/ptp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/sync_e/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/arpcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/igmpcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/pim/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/protocol \
    -I$(ACCTONSRC_DIR)/user/core/l3/ping/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/traceroute/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dns/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2_mgr/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/if_mgr/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/flash/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/syncedrv/include \
    -I$(ACCTONSRC_DIR)/user/process/snmp_proc/include \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1 \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/agent \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/agent/mibgroup \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/agent/mibgroup/mibII \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/agent/mibgroup/rmon \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/include \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/include/net-snmp \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/include/net-snmp/agent \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/include/net-snmp/library \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/user/thirdpty/snmp/net-snmp-5.1/win32

libsnmpmgmt_v3_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_pmgr.c \
    $(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp_trap_queue.c

libsnmpmgmt_v3_la_CPPFLAGS=\
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include

AM_CFLAGS = -Werror -std=gnu89 -Wno-misleading-indentation

#Temporaily workaround for compiler warning
CFLAGS += -Wno-unused-but-set-variable

