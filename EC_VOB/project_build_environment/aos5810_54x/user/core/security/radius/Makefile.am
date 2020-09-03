include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libradius_private.a
lib_LTLIBRARIES = libradius.la

libradius_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/radius/avpair.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/buildreq.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/config.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/ip_util.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/md5.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/sendserver.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_callback.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_init.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_mgr.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_om.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_task.c

libradius_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
	-I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
	-I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
        -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
	-I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
	-I$(ACCTONSRC_DIR)/user/driver/systime/include \
	-I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
	-I$(ACCTONSRC_DIR)/user/process/auth_protocol_proc/include \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/core/mgmt/wtdmgmt/include


libradius_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_pmgr.c \
	$(ACCTONSRC_DIR)/user/core/security/radius/radius_pom.c

libradius_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/security/include

AM_CFLAGS = -Werror -g

#Temporaily workaround for compiler warning
CFLAGS += -Wno-unused-but-set-variable -Wno-format
