include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libprimgmt_private.a

libprimgmt_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/pri_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/pri_mgmt/pri_mgr.c

libprimgmt_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include


lib_LTLIBRARIES = libprimgmt.la

libprimgmt_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/pri_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/pri_mgmt/pri_pmgr.c

libprimgmt_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include
