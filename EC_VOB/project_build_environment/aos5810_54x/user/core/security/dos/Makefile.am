include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdos_private.a
lib_LTLIBRARIES = libdos.la

libdos_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/dos/dos_backdoor.c \
	$(ACCTONSRC_DIR)/user/core/security/dos/dos_init.c \
	$(ACCTONSRC_DIR)/user/core/security/dos/dos_mgr.c

libdos_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/cmgr/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/cfm/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include


libdos_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/dos/dos_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/security/dos/dos_om.c

libdos_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/security/include

CFLAGS += -Werror
