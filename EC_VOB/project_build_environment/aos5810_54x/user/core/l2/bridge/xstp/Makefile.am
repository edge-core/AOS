include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libxstp_private.a

libxstp_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_type.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_backdoor.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_engine.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_engine.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_om_private.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_rx.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_rx.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_uty.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_uty.h

libxstp_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/gvrp/garp \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/gvrp/garp/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/amtrl3/include


lib_LTLIBRARIES = libxstp.la

libxstp_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/xstp_pom.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/xstp/xstp_pom.c

libxstp_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/erps/include 
