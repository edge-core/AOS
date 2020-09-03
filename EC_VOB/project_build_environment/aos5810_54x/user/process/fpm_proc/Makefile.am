include $(top_srcdir)/commdef.mk

bin_PROGRAMS = fpm_proc

fpm_proc_SOURCES = \
    $(ACCTONSRC_DIR)/user/process/fpm_proc/fpm_proc.c

fpm_proc_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCSRC)/user/core/l3/amtrl3/include \
    -I$(ACCSRC)/sysinclude/mibconstants \
    -I$(FRR_SRC_DIR)/fpm

fpm_proc_LDADD = \
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
