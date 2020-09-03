SUBDIRS = dot1x keygen mgmt_ip_flt portauthsrvc portsec sshv2 tacacs userauth radius netaccess webauth appfilter

include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libaaa_private.a

lib_LTLIBRARIES = libaaa.la

libaaa_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/security_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/security/aaa_om.c \
    $(ACCTONSRC_DIR)/user/core/security/aaa_mgr.c

libaaa_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/security/tacacs \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/process/auth_protocol/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/process/auth_protocol_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include

libaaa_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/aaa_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/security/aaa_pom.c

libaaa_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include

AM_CFLAGS = -Werror
