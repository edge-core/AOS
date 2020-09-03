include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libsntp_private.a
lib_LTLIBRARIES =libsntp.la

libsntp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_dbg.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_txrx.c


libsntp_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/process/app_protocol_proc/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include

libsntp_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/sntp/sntp_pmgr.c

libsntp_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
