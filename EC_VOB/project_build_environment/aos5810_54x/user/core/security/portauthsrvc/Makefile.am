include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libportauthsrvc_private.a

libportauthsrvc_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/portauthsrvc/portauthsrvc_mgr.c \
        $(ACCTONSRC_DIR)/user/core/security/portauthsrvc/os/linux/portauthsrvc_os.c

libportauthsrvc_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
  -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
  -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
  -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
  -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
  -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
  -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
  -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
  -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
  -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
  -I$(ACCTONSRC_DIR)/user/core/security/include \
  -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
  -I$(ACCTONSRC_DIR)/user/core/security/portauthsrvc/os/include

AM_CFLAGS = -Werror -g
CFLAGS += -Wno-format
