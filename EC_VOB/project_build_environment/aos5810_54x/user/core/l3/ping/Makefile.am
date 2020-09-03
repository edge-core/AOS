include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libping_private.a

lib_LTLIBRARIES = libping.la


libping_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/ping/include/ping_init.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/include/ping_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/include/ping_om.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/include/ping_task.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/include/ping_type.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/ping_init.c \
	$(ACCTONSRC_DIR)/user/core/l3/ping/ping_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l3/ping/ping_om.c \
	$(ACCTONSRC_DIR)/user/core/l3/ping/ping_om_private.h \
	$(ACCTONSRC_DIR)/user/core/l3/ping/ping_task.c

libping_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ping/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include \
	-I$(ACCTONSRC_DIR)/user/process/app_protocol_proc/include


libping_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/ping/ping_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/ping/ping_pom.c

libping_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l3/ping/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include

CFLAGS += -Werror 