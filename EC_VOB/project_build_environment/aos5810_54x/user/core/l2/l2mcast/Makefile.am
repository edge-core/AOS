include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libl2mcast_private.a
lib_LTLIBRARIES = libl2mcast.la

libl2mcast_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/l2mcast/l2mcast_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/l2mcast/l2mcast_mgr.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_engine.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_init.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_om.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_querier.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_unknown.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_backdoor.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_mgr.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_timer.c

libl2mcast_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/core/qos/include \
        -I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/process/driver_proc/include \
        -I$(ACCTONSRC_DIR)/user/driver/msl/include \
	-I$(ACCTONSRC_DIR)/user/driver/msl/mslt/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
	-I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants/ \
	-I$(ACCTONSRC_DIR)/cmnlib/common \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
        -I$(ACCTONSRC_DIR)/cmnlib/common/include

libl2mcast_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_pmgr.c \
        $(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/mldsnp_pom.c

libl2mcast_la_CPPFLAGS = \
	-I$(OEM_INCLUDEDIR) \
        -I$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/sysinclude \
	-I$(ACCTONSRC_DIR)/sysinclude/mibconstants/ \
 	-I$(ACCTONSRC_DIR)/cmnlib/common/include \
	-I$(ACCTONSRC_DIR)/cmnlib/user/include \
      -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/l2mcast/mldsnp/include

# TBD: Turn off -Werror until warning in IGV3SNP_VLANMemberAdd() is fixed
AM_CFLAGS = -Wno-error
