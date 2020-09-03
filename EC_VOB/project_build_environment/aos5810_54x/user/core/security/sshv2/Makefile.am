include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libssh_private.a
lib_LTLIBRARIES = libssh.la

libssh_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_init.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_task.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_om.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_mgr.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_vm.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshd.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/atomicio.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/servconf.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/canohost.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/compat.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/kex.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/packet.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/mpaux.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/kexdhs.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/kexdh.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/rijndael.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/dh.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/kexgex.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/kexgexs.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/cipher.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/compress.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/channels.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/nchan.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/rsa.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/dispatch.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth-passwd.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth-rsa.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth1.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth2.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth2-passwd.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/session.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/sshpty.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/ttymodes.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/deattack.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/mac.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/match.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/crc32.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/uuencode.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/bufaux.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/buffer.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/key.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/xmalloc.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/misc.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/ssh-rsa.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/ssh-dss.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/fatal.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/log.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/base64.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/hostfile.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/serverloop.c \
    $(ACCTONSRC_DIR)/user/core/security/sshv2/auth2-pubkey.c



libssh_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mgmt_ip_flt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2 \
    -I$(ACCTONSRC_DIR)/user/process/cli_proc/include

libssh_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/sshv2/sshd_pmgr.c

libssh_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/security/include \
	-I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
	-I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include
CFLAGS += -Werror -Wno-misleading-indentation
