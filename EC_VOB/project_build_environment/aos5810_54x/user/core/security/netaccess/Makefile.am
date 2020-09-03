include $(top_builddir)/commdef.mk

noinst_LIBRARIES= libnetaccess_private.a
lib_LTLIBRARIES = libnetaccess.la

libnetaccess_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_init.c \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_mgr.c \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_om.c \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_vm.c \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_backdoor.c

libnetaccess_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
    -I$(ACCTONSRC_DIR)/user/core/security/dot1x \
    -I$(ACCTONSRC_DIR)/user/core/security/portsec \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/rspan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/add/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysctrl/xor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -DNO_CAST \
    -DNO_IDEA \
    -DNO_MD2 \
    -DNO_MD4 \
    -DNO_MDC2 \
    -DNO_RC2 \
    -DNO_RC5 \
    -DNO_RIPEMD \
    -DNO_FP_API

libnetaccess_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/netaccess/netaccess_pmgr.c

libnetaccess_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include
