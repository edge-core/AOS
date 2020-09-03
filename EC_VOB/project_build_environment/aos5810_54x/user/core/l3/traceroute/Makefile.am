include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libtraceroute_private.a

lib_LTLIBRARIES = libtraceroute.la


libtraceroute_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/include/traceroute_init.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/include/traceroute_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/include/traceroute_om.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/include/traceroute_task.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/include/traceroute_type.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_init.c \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_om.c \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_om_private.h \
	$(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_task.c

libtraceroute_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/traceroute/include \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/process/app_protocol_proc/include


libtraceroute_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/l3/traceroute/traceroute_pom.c

libtraceroute_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l3/traceroute/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include

CFLAGS += -Werror -g -Wno-misleading-indentation
