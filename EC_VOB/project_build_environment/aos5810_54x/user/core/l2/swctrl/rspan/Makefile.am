include $(top_builddir)/commdef.mk


noinst_LIBRARIES = librspan_private.a

librspan_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/rspan_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/rspan_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/rspan_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/rspan_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/rspan_mgr.c

librspan_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/core/security/dot1x \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include


lib_LTLIBRARIES = librspan.la

librspan_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/rspan_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/rspan_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/rspan_om.c

librspan_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include
