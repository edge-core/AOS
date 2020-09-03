if WITHOUT_SDK

bin_SCRIPTS = $(top_builddir)/../prebuilt/process/driver_proc/.libs/driver_proc

mylibdir = ${prefix}/../lib/modules/4.19.67/
sdkdir = $(top_builddir)/../prebuilt/sdk
dist_mylib_SCRIPTS = \
    $(sdkdir)/linux-user-bde.ko \
    $(sdkdir)/linux-kernel-bde.ko

else

include $(top_srcdir)/commdef.mk

bin_PROGRAMS = driver_proc drvdbg_proc 
#noinst_PROGRAMS = get_accu_sys_up_time

driver_proc_SOURCES = \
        $(ACCTONSRC_DIR)/user/process/driver_proc/driver_proc.c \
        $(ACCTONSRC_DIR)/user/process/driver_proc/driver_proc_comm.c \
        $(ACCTONSRC_DIR)/user/process/driver_proc/driver_group.c \
        $(ACCTONSRC_DIR)/user/driver/swdrv/swdrv_monitor.c \
        $(ACCTONSRC_DIR)/user/process/driver_proc/driver_group_operation.c \
        $(SDKDRV)/dev_swdrv.c \
        $(SDKDRV)/dev_amtrdrv.c \
        $(SDKDRV)/dev_nicdrv_gateway.c \
        $(SDKDRV)/dev_nicdrv.c \
        $(SDKDRV)/dev_nmtrdrv.c \
        $(SDKDRV)/dev_rm.c \
        $(SDKDRV)/dev_swdrvl4.c \
        $(SDKDRV)/dev_swdrvl3.c

driver_proc_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        $(SDK_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/driver/iscagent/include \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/dhcp/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/include \
        -I$(ACCTONSRC_DIR)/user/driver/fs/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscagent/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
        -I$(ACCTONSRC_DIR)/user/process/driver_proc/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
        -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include

driver_proc_LDADD = \
        $(COMMON_LIBS) \
        $(top_builddir)/apps/backdoor/libbackdoor.la \
        $(ACCPROJ)/cmnlib/libcmnlib.la \
        $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
        $(top_builddir)/core/mgmt/mib2mgmt/libmib2mgmt.la \
        $(top_builddir)/core/l2/l2mux/libl2mux.la \
        $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
        $(top_builddir)/core/l2/swctrl/libtrunk.la \
        $(top_builddir)/core/l2/swctrl/libamtr.la \
        $(top_builddir)/core/l2/swctrl/libswctrl.la \
        $(top_builddir)/core/l2/vlan/libvlan.la \
        $(top_builddir)/core/l3/netcfg/libnetcfg.la \
        $(top_builddir)/driver/fs/libfs.la \
        $(top_builddir)/driver/swdrv/libswdrv.la \
        $(top_builddir)/driver/sysdrv/libsysdrv_private.a \
        $(top_builddir)/driver/sysdrv/libsysdrv.la \
        $(top_builddir)/driver/systime/libsystime.la \
        $(top_builddir)/driver/systime/libsystime_private.a \
        $(top_builddir)/driver/fs/libfs_static.a \
        $(top_builddir)/driver/landrv/liblandrv.la \
        $(top_builddir)/driver/devdrv/libdevdrv.la \
        $(top_builddir)/driver/iscdrv/libiscdrv.la \
        $(top_builddir)/driver/iscagent/libiscagent.la \
        $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
        $(top_builddir)/driver/leddrv/libleddrv.la \
        $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
        $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
        $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
        $(top_builddir)/core/mgmt/sysmgmt/libsysmgmt.la \
        $(top_builddir)/core/mgmt/syslog/libsyslog.la \
        $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
        $(top_builddir)/core/security/appfilter/libappfilter.la \
        $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
        $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
        $(top_builddir)/core/mgmt/telnet/libtelnet.la \
        $(top_builddir)/core/mgmt/dhcpv6/libdhcpv6.la \
        $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
        $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
        $(top_builddir)/driver/devdrv/onlp/libdevdrvonlp_static.a \
        $(top_builddir)/driver/devdrv/onlp/libdevdrvonlp.la \
        $(SDK_LIBS) \
        -lpthread

drvdbg_proc_SOURCES = \
        $(ACCTONSRC_DIR)/user/process/driver_proc/drvdbg_proc.c

drvdbg_proc_CPPFLAGS = \
        $(COMMON_CPPFLAGS)

drvdbg_proc_LDADD = \
        -lpthread


#get_accu_sys_up_time_SOURCES = \
#    $(ACCTONSRC_DIR)/user/driver/systime/get_accu_sys_up_time.c

#get_accu_sys_up_time_CPPFLAGS = \
#    -I$(ACCTONSRC_DIR)/sysinclude \
#    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
#    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
#    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
#    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
#    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
#    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
#    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include

#get_accu_sys_up_time_LDADD = \
#    $(top_builddir)/driver/systime/libsystime_static.a \
#    $(top_builddir)/driver/systime/libsystime.la \
#    $(top_builddir)/driver/fs/libfs.la \
#    $(top_builddir)/driver/fs/libfs_static.a \
#    $(top_builddir)/core/mgmt/sysrsc/libsysrsc.la \
#    $(top_builddir)/core/l2/swctrl/libswctrl.la \
#    $(top_builddir)/core/l2/swctrl/libamtr.la \
#    $(top_builddir)/driver/iscdrv/libiscdrv.la \
#    $(top_builddir)/core/mgmt/ucmgmt/libucmgmt.la \
#    $(top_builddir)/core/l3/netcfg/libnetcfg.la \
#    $(top_builddir)/apps/cli/libcli.la \
#    $(top_builddir)/driver/swdrv/libswdrv.la \
#    $(top_builddir)/driver/devdrv/libdevdrv.la \
#    $(top_builddir)/core/mgmt/sys_callback/libsys_callback.la \
#    $(top_builddir)/core/mgmt/ndsnp/libndsnp.la \
#    $(top_builddir)/core/mgmt/debug/libdebug.la \
#    $(top_builddir)/driver/buffermgmt/libbuffermgmt.la \
#    $(top_builddir)/core/mgmt/stkmgmt/libstkmgmt.la \
#    $(top_builddir)/core/mgmt/cfgdb/libcfgdb.la \
#    $(top_builddir)/core/mgmt/sysctrl/xor/libsysctrl_xor.la \
#    $(top_builddir)/apps/snmpmgmt/v3/libsnmpmgmt_v3.la \
#    $(top_builddir)/core/l2/bridge/xstp/libxstp.la \
#    $(top_builddir)/driver/landrv/liblandrv.la \
#    $(top_builddir)/core/mgmt/syslog/libsyslog.la \
#    $(top_builddir)/driver/sysdrv/libsysdrv.la \
#    $(top_builddir)/driver/phyaddr_access/libphyaddr_access.la \
#    $(top_builddir)/apps/backdoor/libbackdoor.la \
#    $(top_builddir)/core/cmgr/libcmgr.la \
#    $(top_builddir)/core/l2/lldp/liblldp.la \
#    $(top_builddir)/core/l2/swctrl/lacp/liblacp.la \
#    $(top_builddir)/core/l2/swctrl/rspan/librspan.la \
#    $(top_builddir)/core/l2/vlan/libvlan.la \
#    $(top_builddir)/core/mgmt/dhcpv6/libdhcpv6.la \
#    $(top_builddir)/core/mgmt/sflow/libsflow.la \
#    $(top_builddir)/core/mgmt/wtdmgmt/libwtdmgmt.la \
#    $(top_builddir)/core/security/mgmt_ip_flt/libmgmtipflt.la \
#    $(top_builddir)/driver/i2cdrv/libi2cdrv.la \
#    $(top_builddir)/driver/leddrv/libleddrv.la \
#    $(SDKPROJ)/build/unix-user/$(MODEL_NAME)/libbcmsdk.a \
#    $(ACCPROJ)/cmnlib/libcmnlib.la \
#    -lpthread

AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-format

endif # if WITHOUT_SDK