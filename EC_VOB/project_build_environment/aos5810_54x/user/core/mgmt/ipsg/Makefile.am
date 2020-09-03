include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libipsg_private.a

lib_LTLIBRARIES = libipsg.la

libipsg_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_mgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_om.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_backdoor.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_init.c 

libipsg_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stktplg/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include 

libipsg_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ipsg_pom.c
	
libipsg_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
