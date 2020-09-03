include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libsflow_private.a
lib_LTLIBRARIES = libsflow.la

libsflow_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_task.c

libsflow_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/process/sflow_proc/include \
    -I$(ACCTONSRC_DIR)/prom/diag/network_adpt/include \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants

libsflow_la_SOURCES = \
  	$(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sflow/sflow_om.c

libsflow_la_CPPFLAGS = \
  	$(COMMON_CPPFLAGS) \
  	-I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
