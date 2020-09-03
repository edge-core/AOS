include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libextbrg_private.a

libextbrg_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/extbrg_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/extbrg/extbrg_mgr.c

libextbrg_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include


lib_LTLIBRARIES = libextbrg.la

libextbrg_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/extbrg_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/extbrg/extbrg_pmgr.c

libextbrg_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include
