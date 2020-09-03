SUBDIRS = extbrg pri_mgmt xstp

include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libbridge_private.a

libbridge_private_a_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/include/bridge_init.h \
	$(ACCTONSRC_DIR)/user/core/l2/bridge/bridge_init.c

libbridge_private_a_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/l2/bridge/include

libbridge_private_a_LIBADD = \
	extbrg/libextbrg_private.a \
	pri_mgmt/libprimgmt_private.a \
	xstp/libxstp_private.a

AM_CFLAGS = -Werror
