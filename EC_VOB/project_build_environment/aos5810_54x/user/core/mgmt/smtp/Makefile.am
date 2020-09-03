include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsmtp_private.a
lib_LTLIBRARIES =libsmtp.la

libsmtp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_util.c


libsmtp_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/process/core_util_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include

libsmtp_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/smtp/smtp_pmgr.c

libsmtp_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/smtp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
