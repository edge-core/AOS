include $(top_builddir)/commdef.mk

noinst_LIBRARIES   = libnetcfg_private.a
lib_LTLIBRARIES = libnetcfg.la

libnetcfg_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/netcfg_mgr_main.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/netcfg_om_main.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_mgr_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/ip_mgr/ip_mgr.c\
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/netcfg_mgr_route.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/netcfg_om_route.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/netcfg_mgr_nd.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/netcfg_om_nd.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/netcfg_mgr_rip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/netcfg_om_rip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/netcfg_mgr_ospf.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/netcfg_om_ospf.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/icmpcfg/icmpcfg.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/netcfg_mgr_pbr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/netcfg_om_pbr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/nd_mgr/nd_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/route_mgr/route_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/icmp_mgr/icmp_mgr.c

libnetcfg_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/icmpcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp \
    -I$(ACCTONSRC_DIR)/user/core/l3/pim/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/igv3snp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/protocol/ \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/pim/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/bgp/include \
    -I$(ACCTONSRC_DIR)/user/process/netcfg_proc/include

libnetcfg_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/netcfg_pmgr_main.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/netcfg_pmgr_route.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_pmgr_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/netcfg_pom_main.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/netcfg_pom_route.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_pom_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/netcfg_om_ip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/netcfg_pmgr_nd.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/netcfg_pom_nd.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/netcfg_pmgr_rip.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/netcfg_pmgr_ospf.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/netcfg_pmgr_pbr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/netcfg_pom_pbr.c \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/include/ip_lib.h \
    $(ACCTONSRC_DIR)/user/core/l3/netcfg/ip_lib.c

libnetcfg_la_CPPFLAGS=\
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ripcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ospfcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/icmpcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/pbrcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/linux_porting/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/udphelper \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/bgp/include \
    -I$(ACCTONSRC_DIR)/user/protocol/ 

# TBD:Turn off -Werror until the hw_info data type is fixed.
#     Need to revise hw_info as "void*" in netcfg_mgr_pbr.c.
#     NETCFG_OM_PBR_BindingEntry_T.nexthop_hwinfo should be
#     changed as "void*".
AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format
