include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libntp_private.a
lib_LTLIBRARIES =libntp.la

libntp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_dbg.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_recvbuff.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_task.c

libntp_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sntp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/process/app_protocol_proc/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include

libntp_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/ntp/ntp_pmgr.c

libntp_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ntp/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
