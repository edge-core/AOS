include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libcfgdb.la

libcfgdb_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/cfgdb_mgr.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/cfgdb_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/cfgdb_util.c

libcfgdb_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
        -g \
	-I$(ACCTONSRC_DIR)/user/driver/fs/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include

CFLAGS += -Werror

