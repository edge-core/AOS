lib_LTLIBRARIES = libbuffermgmt.la

libbuffermgmt_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/driver/buffermgmt/buffer_mgr.c \
	$(ACCTONSRC_DIR)/user/driver/buffermgmt/buffermgmt_init.c

libbuffermgmt_la_CPPFLAGS = \
	-I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants/ \
 	-I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include

AM_CFLAGS = -Werror
