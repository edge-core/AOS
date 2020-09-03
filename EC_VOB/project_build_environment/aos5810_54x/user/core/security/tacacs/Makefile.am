include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libtacacs_private.a

lib_LTLIBRARIES = libtacacs.la

libtacacs_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_account.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_account_c.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_authen.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_authen_c.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_author.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_author_c.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_md5.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_packet.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tac_utils.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_init.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_mgr.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_om.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_task.c

libtacacs_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
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
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include

libtacacs_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/security/tacacs/tacacs_pom.c

libtacacs_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include

AM_CFLAGS = -Werror -g
CFLAGS += -Wno-format
