include $(top_srcdir)/commdef.mk

bin_PROGRAMS = mac_sync_proc

mac_sync_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/mac_sync_proc/mac_sync_proc.c \
    $(ACCTONSRC_DIR)/user/process/mac_sync_proc/libnetlink.c

mac_sync_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCSRC)/user/core/l3/amtrl3/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCSRC)/sysinclude/mibconstants

mac_sync_proc_LDADD = \
    $(COMMON_LIBS) \
    $(ACCPROJ)/user/core/l2/swctrl/libswctrl.la \
    $(ACCPROJ)/user/core/mgmt/syslog/libsyslog.la \
    $(ACCPROJ)/user/driver/i2cdrv/libi2cdrv.la \
    $(ACCPROJ)/user/core/l2/bridge/xstp/libxstp.la \
    $(ACCPROJ)/user/core/l2/swctrl/libamtr.la \
    $(ACCPROJ)/user/driver/phyaddr_access/libphyaddr_access.la \
    $(ACCPROJ)/user/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
    $(ACCPROJ)/user/core/mgmt/sysrsc/libsysrsc.la

AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format
