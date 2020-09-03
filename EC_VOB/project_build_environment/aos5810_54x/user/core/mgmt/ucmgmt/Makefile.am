include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libucmgmt.la

libucmgmt_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/ucmgmt_init.c \
	$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/uc_mgr.c

libucmgmt_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include

CFLAGS += -Werror
