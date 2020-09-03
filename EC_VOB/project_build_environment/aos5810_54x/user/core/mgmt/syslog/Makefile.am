include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsyslog_private.a
lib_LTLIBRARIES = libsyslog.la

libsyslog_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_backdoor.c

libsyslog_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/process/core_util_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include

libsyslog_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/syslog/syslog_pom.c

libsyslog_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include


AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format

