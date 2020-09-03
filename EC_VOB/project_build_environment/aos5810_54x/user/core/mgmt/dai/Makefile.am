include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdai_private.a

lib_LTLIBRARIES = libdai.la

libdai_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/include/dai_backdoor.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/include/dai_init.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/include/dai_mgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/include/dai_type.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_engine.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_engine.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_om.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_om.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_backdoor.c 


libdai_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/cli \
	-I$(ACCTONSRC_DIR)/user/apps/cli/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
	-I$(ACCTONSRC_DIR)/user/core/qos/acl/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml \
	-I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
	-I$(ACCTONSRC_DIR)/user/process/ip_service_proc/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants

libdai_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/include/dai_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/mgmt/dai/dai_pmgr.c
	
libdai_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include

libdaisnp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
