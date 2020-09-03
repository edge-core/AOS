include $(top_builddir)/commdef.mk

noinst_LIBRARIES   = libvrrp_private.a
lib_LTLIBRARIES = libvrrp.la

libvrrp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_mgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_om.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_init.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_task.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_sys_adpt.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_txrx.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_vm.c 

libvrrp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/vrrp \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/arpcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ndcfg/include \
	-I$(ACCTONSRC_DIR)/cmnlib/include \
        -I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/memmgmt \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/protocol

	
libvrrp_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/vrrp/vrrp_pom.c

libvrrp_la_CPPFLAGS=\
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l3/vrrp/include \
    -I$(ACCTONSRC_DIR)/cmnlib/include 

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
