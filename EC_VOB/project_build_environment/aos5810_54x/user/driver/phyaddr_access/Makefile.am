include $(top_builddir)/commdef.mk

lib_LTLIBRARIES=libphyaddr_access.la

libphyaddr_access_la_SOURCES=$(ACCTONSRC_DIR)/user/driver/phyaddr_access/phyaddr_access.c

libphyaddr_access_la_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include

AM_CFLAGS = -Werror
