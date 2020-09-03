include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libbackdoor.la

libbackdoor_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/apps/backdoor/backdoor_mgr.c \
	$(ACCTONSRC_DIR)/user/apps/backdoor/backdoor_lib.c


libbackdoor_la_CPPFLAGS = \
 	$(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

