include $(top_builddir)/commdef.mk


noinst_LIBRARIES = liblacp_private.a

liblacp_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/lacp_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/lacp_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/lacp_type.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_mkrm.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_muxm.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_rxm.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_txm.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_util.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_util.h

liblacp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include


lib_LTLIBRARIES = liblacp.la

liblacp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/lacp_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/lacp_pom.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/lacp/lacp_pom.c

liblacp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include
