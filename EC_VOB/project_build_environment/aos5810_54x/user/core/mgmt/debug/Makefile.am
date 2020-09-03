include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libdebug.la

libdebug_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/debug/debug_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/debug/debug_om.c

libdebug_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/debug/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/debug

CFLAGS += -Werror
