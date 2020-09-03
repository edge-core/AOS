include $(top_srcdir)/commdef.mk

bin_PROGRAMS = sys_callback_proc

sys_callback_proc_SOURCES = \
	$(ACCTONSRC_DIR)/user/process/sys_callback_proc/sys_callback_proc.c \
	$(ACCTONSRC_DIR)/user/process/sys_callback_proc/sys_callback_group.c \
        $(ACCTONSRC_DIR)/user/process/sys_callback_proc/sys_callback_proc_comm.c

sys_callback_proc_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(SDKKEN)/include \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/process/sys_callback_proc/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/ \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ipsg/include \
        -I$(ACCTONSRC_DIR)/user/core/security/portsec \
        -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
        -I$(ACCTONSRC_DIR)/user/core/security/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include  

sys_callback_proc_LDADD = \
	$(COMMON_LIBS) 

sys_callback_proc_LDADD += \
	$(top_builddir)/apps/backdoor/libbackdoor.la \
        $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
        $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
        $(top_builddir)/core/mgmt/syslog/libsyslog.la \
        $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
        $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
        $(top_builddir)/core/l2/swctrl/libamtr.la \
        $(top_builddir)/core/l2/swctrl/libswctrl.la \
        $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
        $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
        $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
	$(top_builddir)/core/l2/swctrl/rspan/librspan.la \
	$(top_builddir)/core/mgmt/wtdmgmt/libwtdmgmt.la \
	$(top_builddir)/core/security/appfilter/libappfilter.la \
        $(top_builddir)/core/security/portsec/libportsec.la \
        $(top_builddir)/core/security/netaccess/libnetaccess.la \
	$(ACCPROJ)/cmnlib/libcmnlib.la \
	-lpthread

CFLAGS += -Werror
