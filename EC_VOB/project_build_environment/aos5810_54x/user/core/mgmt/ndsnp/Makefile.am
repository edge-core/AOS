include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libndsnp_private.a

lib_LTLIBRARIES = libndsnp.la

libndsnp_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_mgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_om.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_engine.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_backdoor.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_init.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_task.c    

libndsnp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/cmmlib/common/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcpv6/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/prom/loader/u-boot-1.3.2/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include 

libndsnp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/ndsnp_pom.c
	
libndsnp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ndsnp/include

# TBD: Turn off -Werror until all compiele warnings are fixed
AM_CFLAGS = -Wno-error
