SUBDIRS = lacp rspan

include $(top_builddir)/commdef.mk


noinst_LIBRARIES = libswctrl_private.a libamtr_private.a libtrunk_private.a libnmtr_private.a

lib_LTLIBRARIES = libswctrl.la libamtr.la libtrunk.la libnmtr.la


libswctrl_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_init.c \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_task.c \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl.c

libswctrl_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sflow/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/erps/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include

libamtr_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/amtr_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/amtr_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/amtr_om.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/amtr_type.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_mgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_om_private.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_om_private.c

libamtr_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/privatevlan/include

libtrunk_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/trk_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/trk_mgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/trk_init.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/trk_mgr.c

libtrunk_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include

libnmtr_private_a_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/swctrl/nmtr_init.c \
        $(ACCTONSRC_DIR)/user/core/l2/swctrl/nmtr_hist.c \
        $(ACCTONSRC_DIR)/user/core/l2/swctrl/nmtr_mgr.c

libnmtr_private_a_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include


libswctrl_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_om.c \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_pom.c \
    $(ACCTONSRC_DIR)/user/core/l2/swctrl/swctrl_pmgr.c

libswctrl_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include

libamtr_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/amtr_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_om.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/amtr_pom.c

libamtr_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include
 
libtrunk_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/trk_lib.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/include/trk_pmgr.h \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/trk_lib.c \
	$(ACCTONSRC_DIR)/user/core/l2/swctrl/trk_pmgr.c

libtrunk_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include

libnmtr_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/core/l2/swctrl/nmtr_pmgr.c

libnmtr_la_CPPFLAGS = \
        $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format
