include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libip6sg_private.a

lib_LTLIBRARIES = libip6sg.la

libip6sg_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_mgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_om.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_backdoor.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_init.c 

libip6sg_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/cmmlib/common/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include 

libip6sg_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/ip6sg_pom.c
	
libip6sg_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ip6sg/include

# TBD: Turn of -Werror until all compile warnings are fixed
AM_CFLAGS = -Wno-error
