include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libwebauth_private.a

lib_LTLIBRARIES = libwebauth.la

libwebauth_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_init.c \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_mgr.c \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_task.c

libwebauth_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include 

libwebauth_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/security/webauth/webauth_om.c

libwebauth_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include

#Temporaily workaround for compiler warning
CFLAGS += -Wno-unused-but-set-variable -Wno-misleading-indentation

CFLAGS += -Werror
