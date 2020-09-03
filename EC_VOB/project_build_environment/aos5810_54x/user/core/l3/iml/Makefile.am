include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libiml_private.a
lib_LTLIBRARIES = libiml.la

libiml_private_a_SOURCES=\
        $(ACCTONSRC_DIR)/user/core/l3/iml/iml_mgr.c \
        $(ACCTONSRC_DIR)/user/core/l3/iml/mac_cache.c

libiml_private_a_CFLAGS=$(CFLAGS)

libiml_private_a_CPPFLAGS= \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/vrrp \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai\
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/igv3snp/include \
        -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include

libiml_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/iml/iml_pmgr.c

libiml_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include

AM_CFLAGS = -Werror
