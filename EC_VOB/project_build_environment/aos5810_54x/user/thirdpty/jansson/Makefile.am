include $(top_builddir)/commdef.mk

#EXTRA_DIST = jansson.def

#include_HEADERS = jansson.h jansson_config.h

lib_LTLIBRARIES = libjansson.la
#noinst_LIBRARIES = libjansson_private.a

libjansson_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/dump.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/error.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/hashtable.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/hashtable.h \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/hashtable_seed.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/jansson_private.h \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/load.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/lookup3.h \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/memory.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/pack_unpack.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/strbuffer.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/strbuffer.h \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/strconv.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/utf.c \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/utf.h \
	$(ACCTONSRC_DIR)/user/thirdpty/jansson/src/value.c

libjansson_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/thirdpty/jansson/src

CFLAGS = -fno-strict-aliasing -g -w
